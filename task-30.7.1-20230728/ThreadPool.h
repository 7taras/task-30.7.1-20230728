#pragma once
#include <queue>
#include <future>
#include <condition_variable>
#include <vector>

using namespace std;

// удобное определение для сокращения кода
typedef function<void()> task_type;

// тип указатель на функцию, которая является эталоном для функций задач
typedef void (*FuncType) (int*, int, int);

// пул потоков
class ThreadPool 
{
public:
    ThreadPool();
    // запуск:
    void start();
    // остановка:
    void stop();
    // проброс задач
    void push_task(FuncType f, int* array, int begin, int end);
    // функция входа для потока
    void threadFunc(int qindex);
private:
    // количество потоков
    int m_thread_count;
    // потоки
    vector<thread> m_threads;
    // очереди задач для потоков
    vector<BlockedQueue<task_type>> m_thread_queues;
    // для равномерного распределения задач
    int m_index;
};

