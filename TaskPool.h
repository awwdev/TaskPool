//awwdev 2019 https://github.com/awwdev

#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <exception>
#include <vector>
#include <string>

namespace taskpool
{
    enum class TaskStatus { Idle, Startable, Progress };

    //typedefs (function signatures)
    using TaskFnDecl = std::function<void(const std::atomic<bool>& allTasksRunning)>;
    using ThreadFnDecl = std::function<void(
        const std::atomic<bool>& allThreadsRunning, 
        const std::atomic<bool>& allTasksRunning, 
        TaskFnDecl taskThread)
    >;

    class TaskThread final
    {
    public:
        TaskThread(
            const std::atomic<bool>& allThreadsRunning,
            const std::atomic<bool>& allTasksRunning
        );
        //WARNING: a task thread shall not be copied, but it needs to be defined manually due to requirements of std::vector
        //however, any call to copy ctor IS AN ERRROR! 
        //the vector is init in a stable way (reserved mem), so a reallocating will not happen, and hence copy/mov ctors will not be called
        TaskThread(const TaskThread&)            { throw std::runtime_error("TaskThread is copied!"); };
        TaskThread& operator=(const TaskThread&) { throw std::runtime_error("TaskThread is copied!"); };
        TaskThread(TaskThread&&)                 { throw std::runtime_error("TaskThread is moved!");  };
        TaskThread& operator=(TaskThread&&)      { throw std::runtime_error("TaskThread is moved!");  };

    public:
        void setStartable()                                 { m_status.store(TaskStatus::Startable); }
        auto getStatusAtomic() -> std::atomic<TaskStatus>&  { return m_status; }
        auto getStatus() const -> TaskStatus                { return m_status.load(); }

        void setTaskFn(std::function<void()>& fn, const size_t repetition);
        auto getTaskFn() -> TaskFnDecl& { return m_taskFn; } 
        
        bool join();

    private:
        std::atomic<TaskStatus> m_status; //a task has a status (idle, startable, progress), only startable can be started
        TaskFnDecl              m_taskFn; //a task is executed through a stored function object
        std::thread             m_thread; //a task has a thread
    };

    class TaskPool final
    {
    public:
        //the object is designed to have a stable and fixed size of threads, you need to specify count
        TaskPool() = delete;
        explicit TaskPool(const std::int8_t); 
        //the object shall not be copied (due to atomics and threads)
        TaskPool(const TaskPool&) = delete;
        TaskPool& operator=(const TaskPool&) = delete;
        //dtor needed to force join threads when lifetime ends
        ~TaskPool();

    public:
        void stopAllTasks();  //should be used at the end of a second logging
        bool startAllTasks(); //needs to be called continuously because it might not start (it will start when all tasks finished)
        auto areTasksInProgress() const -> bool; //before starting a new second logging, check this for true!!!
        void setTask(const size_t taskIndex, std::function<void()>& taskFn, const size_t repetition);
        auto size() const -> size_t { return m_taskThreads.size(); }

    private:
        std::vector<TaskThread> m_taskThreads; //container of custom classes
        std::atomic<bool> m_allTasksRunning;   //this will stop all tasks from repeating 
        std::atomic<bool> m_allThreadsRunning; //needed to stop and join threads
    };
}