// task-30.7.1-20230728.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <queue>
#include <future>
#include <condition_variable>
#include <vector>
#include <random>

#include "ThreadPool.h"
#include "RequestHandler.h"
#include "BlockedQueue.h"




unsigned int numberOfConcurrentThreads;
mutex mtx;

RequestHandler rh;

// Merges two subarrays of array[].
// First subarray is arr[begin..mid]
// Second subarray is arr[mid+1..end]
void merge(int array[], int const left, int const mid, int const right)
{
    int const subArrayOne = mid - left + 1;
    int const subArrayTwo = right - mid;

    // Create temp arrays
    auto* leftArray = new int[subArrayOne],
        * rightArray = new int[subArrayTwo];

    // Copy data to temp arrays leftArray[] and rightArray[]
    for (auto i = 0; i < subArrayOne; i++)
        leftArray[i] = array[left + i];
    for (auto j = 0; j < subArrayTwo; j++)
        rightArray[j] = array[mid + 1 + j];

    auto indexOfSubArrayOne = 0, indexOfSubArrayTwo = 0;
    int indexOfMergedArray = left;

    // Merge the temp arrays back into array[left..right]
    while (indexOfSubArrayOne < subArrayOne && indexOfSubArrayTwo < subArrayTwo)
    {
        if (leftArray[indexOfSubArrayOne] <= rightArray[indexOfSubArrayTwo])
        {
            array[indexOfMergedArray] = leftArray[indexOfSubArrayOne];
            indexOfSubArrayOne++;
        }
        else
        {
            array[indexOfMergedArray] = rightArray[indexOfSubArrayTwo];
            indexOfSubArrayTwo++;
        }
        indexOfMergedArray++;
    }

    // Copy the remaining elements of left[], if there are any
    while (indexOfSubArrayOne < subArrayOne)
    {
        array[indexOfMergedArray] = leftArray[indexOfSubArrayOne];
        indexOfSubArrayOne++;
        indexOfMergedArray++;
    }

    // Copy the remaining elements of right[], if there are any
    while (indexOfSubArrayTwo < subArrayTwo)
    {
        array[indexOfMergedArray] = rightArray[indexOfSubArrayTwo];
        indexOfSubArrayTwo++;
        indexOfMergedArray++;
    }
    delete[] leftArray;
    delete[] rightArray;
}

// begin is for left index and end is right index of the sub-array of arr to be sorted
void mergeSort(int array[], int const begin, int const end)
{
    if (begin >= end)
        return;

    int mid = begin + (end - begin) / 2;

    mtx.lock();
    if (numberOfConcurrentThreads)
    {
        numberOfConcurrentThreads--;
        mtx.unlock();
        future<void> fut = async(launch::async, [&]() { mergeSort(array, begin, mid); });
        mergeSort(array, mid + 1, end);
        fut.get();
    }
    else
    {
        mtx.unlock();
        mergeSort(array, begin, mid);
        mergeSort(array, mid + 1, end);
    }
    merge(array, begin, mid, end);
    return;
}

void mergeSortWithThreadPool(int* arr, int begin, int end)
{
    if (begin >= end)
        return;
    
    int mid = begin + (end - begin) / 2;
    rh.pushRequest(mergeSortWithThreadPool, arr, begin, mid);
    rh.pushRequest(mergeSortWithThreadPool, arr, mid + 1, end);
    
    merge(arr, begin, mid, end);

    return;
}

void mergeSortWithoutThreads(int array[], int const begin, int const end)
{
    if (begin >= end)
        return;

    int mid = begin + (end - begin) / 2;
    mergeSortWithoutThreads(array, begin, mid);
    mergeSortWithoutThreads(array, mid + 1, end);
    merge(array, begin, mid, end);
    return;
}

// Function to print an array
void printArray(int array[], int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << array[i] << " ";
    }
    cout << endl;
}

int main()
{
    numberOfConcurrentThreads = std::thread::hardware_concurrency();
    std::cout << numberOfConcurrentThreads << " concurrent threads are supported.\n";
    //numberOfConcurrentThreads--;

    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 500000);
        
    long arr_size = 4;
    int* arr = new int[arr_size];
    for (long i = 0; i < arr_size; i++)
    {
        arr[i] = dist(rd);
    }

    //cout << "Given array is \n";
    //printArray(arr, arr_size);

    time_t start, end;

    time(&start);
    mergeSort(arr, 0, arr_size - 1);
    time(&end);

    int seconds = int(difftime(end, start));
    std::cout << "Threads. The time: " << seconds << " seconds\n";

    time(&start);
    mergeSortWithoutThreads(arr, 0, arr_size - 1);
    time(&end);

    seconds = int(difftime(end, start));
    std::cout << "No threads. The time: " << seconds << " seconds\n";

    time(&start);
    mergeSortWithThreadPool(arr, 0, arr_size - 1);
    time(&end);

    seconds = int(difftime(end, start));
    std::cout << "Thread pool. The time: " << seconds << " seconds\n";

    //cout << "\nSorted array is \n";
    //printArray(arr, arr_size);
}