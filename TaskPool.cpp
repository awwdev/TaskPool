//awwdev 2019 https://github.com/awwdev

#include "TaskPool.h"

namespace taskpool
{
    TaskThread::TaskThread(
        const std::atomic<bool>& allThreadsRunning,
        const std::atomic<bool>& allTasksRunning)
        : m_status{ TaskStatus::Idle }
        , m_taskFn{}
        , m_thread{}
    {
        //this is how the thread function looks like, just running and eventually doing a task
        const auto threadFn = [](
            const std::atomic<bool>& allThreadsRunning,
            const std::atomic<bool>& allTasksRunning,
            TaskThread& taskThread)
        {
            while (allThreadsRunning.load() == true) {
                //only run the task (+repetitions) when there is a task assigned and it was set to startable
                if (taskThread.getTaskFn() && taskThread.getStatus() == TaskStatus::Startable)
                {
                    taskThread.getStatusAtomic().store(TaskStatus::Progress);
                    taskThread.getTaskFn()(allTasksRunning);
                    taskThread.getStatusAtomic().store(TaskStatus::Idle);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //necessary or cpu will explode
            }
        };
        //finally create the thread with the above function 
        m_thread = std::thread(
            threadFn,
            //params
            std::ref(allThreadsRunning),
            std::ref(allTasksRunning),
            std::ref(*this)
        );
    }

    void TaskThread::setTaskFn(std::function<void()>& fn, const size_t repetition)
    {
        //the task function which is passed here is wrappend in a lambda, because this will  manage repetition and break condition
        const TaskFnDecl _fn = [&, fn, repetition](const std::atomic<bool>& allTasksRunning) {
            for (auto i=0; i < repetition; ++i) {
                if (allTasksRunning) {
                    fn();
                }
                else break; 
            }
        };
        m_taskFn = _fn; //assign the task
        m_status = TaskStatus::Idle; //ensure its idle in the beginning
    }

    auto TaskThread::join() -> bool
    {
        if (m_thread.joinable()) {
            m_thread.join();
            return true;
        }
        return false;
    }

    TaskPool::TaskPool(const std::int8_t maxCount)
        : m_taskThreads      {}
        , m_allTasksRunning  { false }
        , m_allThreadsRunning{ true  }
    {
        m_taskThreads.reserve(maxCount); //stability
        for (int i = 0; i < maxCount; ++i) {
            m_taskThreads.emplace_back(m_allThreadsRunning, m_allTasksRunning); //in place ctor, forwarding ctor args
        }
    }

    TaskPool::~TaskPool()
    {
        m_allThreadsRunning.store(false);
        stopAllTasks();
        int threadsJoined = 0;
        while (m_taskThreads.size() != threadsJoined) {
            for (auto it = m_taskThreads.begin(); it != m_taskThreads.end(); ++it) {
                if (it->join())
                    threadsJoined++;
            }
        }
    }

    void TaskPool::stopAllTasks()
    {
        m_allTasksRunning.store(false);
    }

    auto TaskPool::startAllTasks() -> bool
    {
        if (areTasksInProgress()) return false;
        for (auto& taskThread : m_taskThreads) {
            if (taskThread.getTaskFn()) {
                taskThread.setStartable();
            }
        }
        m_allTasksRunning = true;
        return true;
    }

    auto TaskPool::areTasksInProgress() const -> bool
    {
        for (const auto& taskThread : m_taskThreads) {
            if (taskThread.getStatus() == TaskStatus::Progress)
                return true;
        }
        return false;
    }

    void TaskPool::setTask(const size_t taskIndex, std::function<void()>& taskFn, const size_t repetition)
    {
        if (taskIndex > m_taskThreads.size()) throw std::logic_error("taskIndex bigger than taskPool size");
        m_taskThreads[taskIndex].setTaskFn(taskFn, repetition);
    }

}