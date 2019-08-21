//awwdev 2019 https://github.com/awwdev

#include "TaskPool.h"

#include <iostream>
#include <mutex>
#include <atomic>
#include <functional>

int main()
{
    {
        taskpool::TaskPool taskPool(10);

        std::mutex mtx;
        std::function<void()> fn1 = [&mtx]() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "I am fn1\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
        std::function<void()> fn2 = [&mtx]() {
            static int i { 0 };
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "I am fn2 and I count " << i << "\n";
                ++i;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(70));
        };

        taskPool.setTask(0, fn1, 50);
        taskPool.setTask(1, fn2, 100);

        taskPool.startAllTasks();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        taskPool.stopAllTasks();

        if (!taskPool.areTasksInProgress()) {
            std::cout << "stopped all tasks\n";
        }
    }

    std::cout << "dtor and join success\n";

    system("pause");
}