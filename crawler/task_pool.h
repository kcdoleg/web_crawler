#pragma once

#include <vector>
#include <condition_variable>
#include <mutex>
#include <memory>


/* Main class to manage multithreading process of workers those can take one task,
 * put one task and wait untill pool is empty or wait untill pool is non empty.
 */
template<class Task>
class TaskPool
{
public:
    void addTask(const Task& task)
    {
        m_tasks.push_back(task);
        m_nonEmptyCondition.notify_one();
    }

    Task getTask()
    {
        Task task(std::move(m_tasks.back()));
        m_tasks.pop_back();
        if (m_tasks.empty()) {
            m_emptyCondition.notify_one();
        }
        return std::move(task);
    }

    bool empty() const
    {
        return m_tasks.empty();
    }
    
    std::mutex& getMutex()
    {
        return m_mutex;
    }

    std::condition_variable& getEmptyCondition()
    {
        return m_emptyCondition;
    }

    std::condition_variable& getNonEmptyCondition()
    {
        return m_nonEmptyCondition;
    }

private:
    std::vector<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_emptyCondition;
    std::condition_variable m_nonEmptyCondition;
};
