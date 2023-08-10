#include "ThreadPool.h"

ThreadPool::ThreadPool() :
    m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4),
    m_thread_queues(m_thread_count) 
{

}

void ThreadPool::start()
{
    for (int i = 0; i < m_thread_count; i++) 
    {
        m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
    }
}

void ThreadPool::stop()
{

    for (int i = 0; i < m_thread_count; i++) 
    {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) 
    {
        t.join();
	}
    m_work = false;
    m_event_holder.notify_all();
    for (auto& t : m_threads) 
	{
        if (t.joinable())
        {
            t.join();
        }      
    }
}

void ThreadPool::push_task(FuncType f, int* array, int begin, int end)
{

    // вычисляем индекс очереди, куда положим задачу
    int queue_to_push = m_index++ % m_thread_count;
    // формируем функтор
    task_type task = [=] {f(id, arg); };
    // кладем в очередь
    m_thread_queues[queue_to_push].push(task);

    lock_guard<mutex> l(m_locker);
    task_type new_task([=] {f(array, begin, end); });
    m_task_queue.push(new_task);
    // оповещаем случайный поток о новой задаче
    m_event_holder.notify_one();

}

void ThreadPool::threadFunc(int qindex)
{
    while (true) 
    {
        // обработка очередной задачи
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) 
        {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) 
        {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) 
        {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do) 
        {
            return;
        }
        // выполняем задачу
        task_to_do();
    }
}