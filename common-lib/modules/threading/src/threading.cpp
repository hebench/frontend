// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "../include/threading.h"

#include <chrono>
#include <csignal>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef __linux__
#define THREAD_INTERRUPT_SIGNAL SIGUSR1
#else
#define THREAD_INTERRUPT_SIGNAL SIGINT
#endif

using namespace hebench::Threading;

//------------------------------------------------
// class Task
//------------------------------------------------

class thread_interrupted : public std::runtime_error
{
public:
    thread_interrupted(const std::string &msg) :
        std::runtime_error(msg) {}

    thread_interrupted(const thread_interrupted &src) :
        std::runtime_error(src.what()) {}

    thread_interrupted &operator=(const thread_interrupted &)
    {
        return *this; // exceptions do not copy
    }
};

void Task::signalHandler(int s)
{
    if (s == THREAD_INTERRUPT_SIGNAL)
    {
        std::signal(THREAD_INTERRUPT_SIGNAL, Task::signalHandler);
        throw thread_interrupted("Thread interrupted with signal " + std::to_string(s) + ".");
    }
}

Task::Task() :
    m_bdisposed(false)
{
    // initialize the thread and start it

    m_worker = [](std::shared_ptr<bool> p_bstart, std::shared_ptr<bool> p_bstop, std::shared_ptr<std::mutex> p_mutex_start,
                  std::shared_ptr<std::condition_variable> p_cv_start, std::shared_ptr<std::mutex> p_mutex_working,
                  std::shared_ptr<std::function<void(void)>> p_func) -> void {
        std::signal(THREAD_INTERRUPT_SIGNAL, Task::signalHandler);

        try
        {
            if (!p_bstart)
                throw std::invalid_argument("p_bstart");
            if (!p_bstop)
                throw std::invalid_argument("p_bstop");
            if (!p_mutex_start)
                throw std::invalid_argument("p_mutex_start");
            if (!p_cv_start)
                throw std::invalid_argument("p_cv_wait");
            if (!p_mutex_working)
                throw std::invalid_argument("p_mutex_working");
            if (!p_func)
                throw std::invalid_argument("p_func");

            bool &b_start                     = *p_bstart;
            bool &b_stop                      = *p_bstop;
            std::mutex &mutex_start           = *p_mutex_start;
            std::condition_variable &cv_start = *p_cv_start;
            std::mutex &mutex_working         = *p_mutex_working;
            std::function<void(void)> &func   = *p_func;

            while (!b_stop)
            {
                // wait for wake-up signal

                std::unique_lock<std::mutex> lock_start(mutex_start);
                while (!b_start)
                {
                    cv_start.wait(lock_start);
                }

                // wake signal received

                // lock busy mutex
                std::unique_lock<std::mutex> lock_busy(mutex_working);

                // signal that wake-up is complete
                b_start = false;
                cv_start.notify_all();
                lock_start.unlock(); // let calling thread run

                // wake-up complete: do work
                if (!b_stop)
                {
                    if (func)
                        func();
                } // end if
                // busy mutex unlocked automatically here
            } // end while
        }
        catch (...)
        {
        }
    };

    // start the thread
    m_pthread = startThread();
    // run a simple task for initialization
    this->run<void>([]() -> void { return; }).wait();
}

Task::~Task()
{
    try
    {
        destroy();
    }
    catch (...)
    {
        // do not throw in destructor
    }
}

std::shared_ptr<std::thread> Task::startThread()
{
    m_p_bstart        = std::make_shared<bool>();
    m_p_bstop         = std::make_shared<bool>();
    m_p_mutex_start   = std::make_shared<std::mutex>();
    m_p_cv_start      = std::make_shared<std::condition_variable>();
    m_p_mutex_working = std::make_shared<std::mutex>();
    m_p_func          = std::make_shared<std::function<void(void)>>();

    *m_p_bstart = false;
    *m_p_bstop  = false;

    return std::make_shared<std::thread>(m_worker, m_p_bstart, m_p_bstop, m_p_mutex_start, m_p_cv_start, m_p_mutex_working, m_p_func);
}

void Task::destroy()
{
    // wait for any Run() call to complete
    std::lock_guard<std::mutex> lock(m_mutex_run_monitor);

    try
    {
        // wait for any current run of thread to complete
        wait();
    }
    catch (...)
    {
        // avoid exceptions on destroy()
    }

    // give signal to thread to terminate
    *m_p_bstop = true;
    startTask(nullptr);

    // wait for underlying thread to complete
    if (m_pthread->joinable())
        m_pthread->join();

    // m_mutex_run_monitor automatically unlocked here
}

void Task::startTask(const std::function<void(void)> &f)
{
    wait(); // wait for task to be ready

    *m_p_func = f;

    // send wake-up signal to thread
    std::unique_lock<std::mutex> lock_start(*m_p_mutex_start);
    *m_p_bstart = true;
    m_p_cv_start->notify_all();
    // wait until thread wakes up (thread will set flag back to false)
    while (*m_p_bstart)
        m_p_cv_start->wait(lock_start);
    // lock released automatically on scope exit
}

bool Task::isBusy()
{
    std::unique_lock<std::mutex> lock_busy(*m_p_mutex_working, std::defer_lock);
    return !lock_busy.try_lock();
}

void Task::wait()
{
    // check if thread is alive
    if (!m_pthread->joinable()) // thread did not start or completed running
    {
        // error accessing system thread
        throw std::runtime_error("Task::wait(): Underlying thread is not running.");
    }

    // block untile current task is complete
    std::lock_guard<std::mutex> lock_busy(*m_p_mutex_working);

    // mutex_working lock released here automatically
}

std::shared_ptr<std::thread> Task::detach()
{
    // avoid calls to run during detaching
    std::lock_guard<std::mutex> lock(m_mutex_run_monitor);

    std::shared_ptr<std::thread> retval = m_pthread;

    // send stop signal to thread
    *m_p_bstop = true;
    if (!this->isBusy())
        startTask(nullptr);

    // start a new thread (this will detach from current thread)
    m_pthread = startThread();
    // run a simple task for initialization
    this->run<void>([]() -> void { return; }).wait();

    return retval;
}

bool Task::interrupt()
{
    bool retval = this->isBusy();

    if (retval)
    {
        // task is running, so, terminate thread

        // avoid calls to run() while thread is not in a stable state
        {
            std::lock_guard<std::mutex> lock_run(m_mutex_run_monitor);

#ifdef __linux__
            // stop thread
            *(this->m_p_bstop) = true;

            auto pid = m_pthread->native_handle();
            m_pthread->detach();
            m_pthread.reset(); // destroy thread

            pthread_kill(pid, THREAD_INTERRUPT_SIGNAL);
#else
            throw std::runtime_error("Task::interrupt() is not supported in this environment.");
#endif
            // wait for thread to finish
            std::unique_lock<std::mutex> lock_busy(*(this->m_p_mutex_working), std::defer_lock);
            // wake thread run if idle
            std::unique_lock<std::mutex> lock_start(*m_p_mutex_start);
            *m_p_bstart = true;
            m_p_cv_start->notify_all();
            while (*m_p_bstart)
                m_p_cv_start->wait(lock_start);
            lock_start.unlock();
            // wait for thread to finish
            lock_busy.lock();
            lock_busy.unlock();
            // restart thread
            *(this->m_p_bstop) = false;
            m_pthread          = startThread();
        }
        // run a simple task for initialization
        this->run<void>([]() -> void { return; }).wait();
    }

    return retval;
}

//------------------------------------------------
// class ThreadPool
//------------------------------------------------

void ThreadPool::addElementInternal()
{
    std::lock_guard<std::mutex> lock(m_mutex_monitor);
    Base::addElement(std::make_shared<Task>());
}

Task::Ptr ThreadPool::pop()
{
    std::lock_guard<std::mutex> lock(m_mutex_monitor);
    return Base::pop();
}

void ThreadPool::push(const Task::Ptr &value)
{
    std::lock_guard<std::mutex> lock(m_mutex_monitor);
    Base::push(value);
}

void ThreadPool::addElement(const Task::Ptr &value)
{
    (void)value;
    throw std::logic_error("ThreadPool::addElement(): Cannot add new threads to a thread pool.");
}

//------------------------------------------------
// class StaticThreadPool
//------------------------------------------------

StaticThreadPool::StaticThreadPool(std::size_t size)
{
    // initialize and start all threads in the pool if requested
    for (std::size_t i = 0; i < size; i++)
        addElementInternal();
}

void StaticThreadPool::clear()
{
    throw std::logic_error("ThreadPool::clear(): Static thread pools cannot be cleared.");
}

//------------------------------------------------
// class DynamicThreadPool
//------------------------------------------------

DynamicThreadPool::DynamicThreadPool(std::size_t size) :
    m_maxSize(size)
{
    if (m_maxSize == 0)
        m_maxSize = std::numeric_limits<std::size_t>::max();
}

std::size_t DynamicThreadPool::availableCount() const
{
    // compute available count
    return Base::availableCount() + m_maxSize - count();
}

Task::Ptr DynamicThreadPool::pop()
{
    std::lock_guard<std::mutex> lock(m_mutexMonitor);

    if (Base::availableCount() == 0 && this->availableCount() > 0)
        addElementInternal(); // add element if we still have space

    return Base::pop();
}
