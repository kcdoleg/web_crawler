#pragma once

#include "task_pool.h"

#include <condition_variable>
#include <mutex>
#include <vector>
#include <memory>
#include <iostream>


template<class ThreadContext>
bool isWorkDone(ThreadContext& threadContext)
{
    std::unique_lock<std::mutex> contextLock(threadContext.getMutex());
    return threadContext.isWorkDone();
}


/* Implementation of typical worker:
 * 1) Get one task from task pool
 * 2) Run real working execution of certain type for this task
 * 3) Store result to multiple(maybe empty) pools
 */
template<class ThreadContext, class ExecuteImplFunc, class Task, class... Result>
void executeTasks(
    std::shared_ptr<ThreadContext> threadContext,
    ExecuteImplFunc executeImpl,
    std::shared_ptr<TaskPool<Task>> taskPool,
    std::shared_ptr<TaskPool<Result>>... resultPools)
{
    while (true) {
        // Check if we need exit
        if (isWorkDone(*threadContext)) {
            return;
        }

        // Get task from tasksPool
        std::unique_lock<std::mutex> taskLock(taskPool->getMutex());
        while (taskPool->empty()) {
            // Check if we need exit when we are waiting condition
            if (isWorkDone(*threadContext)) {
                return;
            }
            taskPool->getNonEmptyCondition().wait(taskLock);
        }
        Task task(taskPool->getTask());
        taskLock.unlock();
        
        // Execute one task and produce results
        executeImpl(threadContext, task, resultPools...);
    }    
}

