// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __THREADING_Thread_Pool_H_7e5fa8c2415240ea93eff148ed73539b
#define __THREADING_Thread_Pool_H_7e5fa8c2415240ea93eff148ed73539b

#include <condition_variable>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "../../general/include/nocopy.h"
#include "../../general/include/pool.h"

namespace hebench {
namespace Threading {

/**
 * @brief Encapsulates a task to run on a separate thread.
 * @details Executing a function from an existing Task object has very
 * little overhead because a Task's underlying thread is setup
 * only once and remains active during the life of the object.
 */
class Task
{
public:
    DISABLE_COPY(Task)

    typedef std::shared_ptr<Task> Ptr;

    /**
     * @brief Instantiates a new Task and launches its thread in sleep mode.
     * @return Smart pointer to newly created task.
     * @sa Task::Task
     */
    static Task::Ptr create() { return std::make_shared<Task>(); }

    /**
     * @brief Instantiates a new Task and launches its thread in sleep mode.
     * @details Underlying thread will exist until destruction.
     */
    Task();
    /**
     * @brief Shuts down the thread and frees resources.
     * @details This will wait for underlying thread to join before terminating.
     */
    virtual ~Task();

    template <typename RetType, typename Function, typename... ArgTs>
    /**
     * @brief Initiates an asynchronous operation by running the specified
     * function asynchronously.
     * @param f[in] Function to run. It must be of type RetType(f)(...).
     * @param params Parameters to pass to function \p f. Any parameters that
     * \p f takes by reference must be wrapped by std::ref or std::cref.
     * @returns A std::future<RetType> object that can be queried for completion
     * status and return value of the task.
     * @details If the task is already performing another operation, this
     * method blocks until the running operation completes. Otherwise,
     * the new operation is initiated by launching the function
     * asynchronously and this method returns immediately.
     *
     * Since underlying thread is always running, there is no thread setup
     * overhead. The only overhead is for the system to schedule the thread
     * after the thread wakes up.
     *
     * Any exception thrown by \p f will be stored in the shared state of the
     * returned std::future<RetType> token.
     *
     * @example
     * @code
     * #include <future>
     * ...
     * std::future<int> result =
     * p_task->run<int>(
     *      []()->int
     *      {
     *          return 42;
     *      });
     * std::cout << result.get();
     * @endcode
     */
    std::future<RetType> run(Function &&f, ArgTs &&... params)
    {
        std::lock_guard<std::mutex> lock(this->m_mutex_run_monitor); // only one instance of Run() at a time

        auto action                                            = std::bind(std::forward<Function>(f), std::forward<ArgTs>(params)...);
        std::shared_ptr<std::promise<RetType>> p_local_promise = std::make_shared<std::promise<RetType>>();
        std::future<RetType> result                            = p_local_promise->get_future();
        std::function<void(void)> func                         = [p_local_promise, action, this]() {
            try
            {
                this->retrieveRunReturnValue(*p_local_promise, action);
            }
            catch (...)
            {
                p_local_promise->set_exception(std::current_exception());
            }
        };
        // start the task
        startTask(func);

        return result;
    }

    /**
     * @brief Detaches the underlying std::thread from this Task.
     * @return The detached std::thread object.
     * @details If the detached thread object is running, it will
     * terminate when the running function completes.
     *
     * This task will create and own a new std::thread for its
     * operations.
     */
    std::shared_ptr<std::thread> detach();

    /**
     * @brief Interrupts a running task if any.
     * @retval true if a task was interrupted.
     * @retval false otherwise.
     * @details This implementation is OS dependent. Current implementation
     * has been tested with Ubuntu 16.04. The behavior of this method
     * is undefined when used with any other OS.
     */
    bool interrupt();

    /**
     * @brief Retrieves the ID of the task's underlying thread.
     * @return Standard C++ thread ID object representing the underlying thread.
     */
    std::thread::id getID() const
    {
        if (m_pthread)
            return m_pthread->get_id();
        else
            return std::thread::id();
    }

private:
    static void signalHandler(int s);
    bool isBusy(); // true if a task is running
    void wait(); // blocks until the current running task completes
    void destroy();
    std::shared_ptr<std::thread> startThread();

    // Performs the handshake with underlying thread. If another task is
    // already running, this will wait until that task completes. When this
    // method returns, the next task is already running.
    void startTask(const std::function<void(void)> &f);

    template <typename RetT, class T>
    void retrieveRunReturnValue(std::promise<RetT> &pr, T &action);
    template <class T>
    void retrieveRunReturnValue(std::promise<void> &pr, T &action);

    std::shared_ptr<std::thread> m_pthread; // underlying thread

    std::mutex m_mutex_run_monitor; // monitor for Run() method

    std::shared_ptr<bool> m_p_bstart; // flags thread to start working (true) and flags caller
        // that thread woke up (false)
    std::shared_ptr<bool> m_p_bstop; // used to flag a thread to stop:
        // if true, the thread ends when flagged to run;
        // remains false while task is active
    std::shared_ptr<std::mutex> m_p_mutex_start; // used by thread to sleep until signaled to start
    std::shared_ptr<std::condition_variable> m_p_cv_start; // used to signal the thread to start working
    std::shared_ptr<std::mutex> m_p_mutex_working; // locked while the task is running an operation
    std::shared_ptr<std::function<void(void)>> m_p_func; // function to be called by thread on each run

    std::function<void(std::shared_ptr<bool>, std::shared_ptr<bool>, std::shared_ptr<std::mutex>, std::shared_ptr<std::condition_variable>,
                       std::shared_ptr<std::mutex>,
                       std::shared_ptr<std::function<void(void)>>)>
        m_worker; // internal function running in thread

    bool m_bdisposed;
};

template <typename RetT, class T>
inline void Task::retrieveRunReturnValue(std::promise<RetT> &pr, T &action)
{
    pr.set_value(action());
}

template <class T>
inline void Task::retrieveRunReturnValue(std::promise<void> &pr, T &action)
{
    action();
    pr.set_value();
}

/**
 * @brief Encapsulates a thread pool.
 * @details Thread pools are thread safe. Guaranteed no data races when
 * requesting threads or returning threads to the pool.
 */
class ThreadPool : public hebench::Common::SimplePool<Task::Ptr>
{
public:
    virtual ~ThreadPool() {}

    Task::Ptr pop() override;
    void push(const Task::Ptr &value) override;

    /**
     * @brief Returns whether the pool has all threads added.
     * @return This is always true for a static thread pool.
     */
    virtual bool isAtCapacity() const = 0;

protected:
    virtual void addElementInternal();

    /**
     * @brief Throws std::invalid_argument. Cannot add new threads to a thread
     * pool.
     * @details Thread pools manage their own threads.
     * @throws std::invalid_argument always.
     */
    void addElement(const Task::Ptr &value) override;

private:
    using Base = SimplePool<Task::Ptr>;

    std::mutex m_mutex_monitor; // controls access to the underlying thread pool
        // structure; implementation of thread safety
};

/**
 * @brief Encapsulates a statically allocated thread pool.
 * @details Static thread pools allocate and start all of its
 * threads during initialization.
 */
class StaticThreadPool : public ThreadPool
{
public:
    DISABLE_COPY(StaticThreadPool)

    /**
     * @brief Constructs a new thread pool.
     * @param size Maximum number of threads to hold in the pool.
     */
    StaticThreadPool(std::size_t size);
    virtual ~StaticThreadPool() {}

    /**
     * @brief Returns true. Static thread pool is always at capacity.
     * @return true.
     */
    virtual bool isAtCapacity() const { return true; }

protected:
    void clear() override;

private:
    using Base = ThreadPool;
};

/**
 * @brief Encapsulates a dynamically allocated thread pool.
 * @details Dynamic thread pools allocate new threads on demand if there are
 * none available in the pool.
 */
class DynamicThreadPool : public ThreadPool
{
public:
    DISABLE_COPY(DynamicThreadPool)

    /**
     * @brief Constructs a new thread pool.
     * @param size Maximum number of threads to hold in the pool.
     * @details If \p size is 0, the pool will hold an unlimited amount of
     * threads up to available resources (this may impact performance as the
     * number of threads grows).
     */
    DynamicThreadPool(std::size_t size = 0);
    virtual ~DynamicThreadPool() {}

    std::size_t availableCount() const override;
    Task::Ptr pop() override;

    /**
     * @brief Returns whether the pool has all threads added.
     * @return true if number of threads owned by this thread pool
     * equals the size passed during construction, false otherwise or
     * size was 0.
     */
    bool isAtCapacity() const { return m_maxSize == count(); }

private:
    using Base = ThreadPool;

    std::size_t m_maxSize;
    std::mutex m_mutexMonitor; // controls access to the underlying thread pool
        // structure; implementation of thread safety
};

} // end namespace Threading
} // namespace hebench

#endif // defined __THREADING_Thread_Pool_H_7e5fa8c2415240ea93eff148ed73539b
