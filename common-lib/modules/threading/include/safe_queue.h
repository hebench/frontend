// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _THREADING_SafeQueue_H_7e5fa8c2415240ea93eff148ed73539b
#define _THREADING_SafeQueue_H_7e5fa8c2415240ea93eff148ed73539b

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace hebench {
namespace Threading {

template <class T>
class SafeQueueGuard;

template <class T>
/**
 * @brief Exposes methods to manipulate a safe queue.
 * @details SafeQueueGuard objects return objects of this type when
 * locking succeeds. A SafeQueue object can only be used by the
 * thread that successfully acquired the lock from the SafeQueueGuard.
 * Using the SafeQueue object for another thread causes exceptions.
 */
class SafeQueue
{
public:
    friend class SafeQueueGuard<T>;

    ~SafeQueue();

    SafeQueue(const SafeQueue &src) = delete;
    SafeQueue(SafeQueue &&src);
    SafeQueue &operator=(const SafeQueue &src) = delete;
    SafeQueue &operator                        =(SafeQueue &&src);

    bool empty() const;
    std::size_t size() const;
    typename std::queue<T>::reference front();
    typename std::queue<T>::const_reference front() const;
    typename std::queue<T>::reference back();
    typename std::queue<T>::const_reference back() const;
    void push(const typename std::queue<T>::value_type &val);
    void push(typename std::queue<T>::value_type &&val);
    void pop();
    void swap(SafeQueue &x);
    bool valid() const;

    template <class... Args>
    void emplace(Args &&... args)
    {
        m_pq->emplace(args...);
    }

private:
    SafeQueue(std::mutex &mutex);
    void terminate();
    void prepare(const std::thread::id &owner_th, std::queue<T> *queue, SafeQueueGuard<T> *owner, std::function<void()> on_push,
                 std::function<void()> on_pop, std::function<void()> on_empty, std::function<void()> on_not_empty);

    void validate(const char *func = nullptr) const;

    std::mutex *m_mutex;
    std::unique_lock<std::mutex> m_monitor;

    std::queue<T> *m_pq;
    SafeQueueGuard<T> *m_powner;

    std::thread::id m_th_id;

    std::function<void()> m_on_push;
    std::function<void()> m_on_pop;
    std::function<void()> m_on_empty;
    std::function<void()> m_on_not_empty;
};

template <class T>
class SafeQueueGuard
{
public:
    SafeQueueGuard();
    ~SafeQueueGuard();

    /**
     * @brief Frees resources and notifies all waiting callers to stop waiting.
     * The resulting SafeQueue::valid() will be false.
     */
    void terminate();

    /**
     * @brief Blocks the queue guard from allowing further locks.
     * @details Use unblock() to allow further locks.
     *
     * After this method returns, further calls to locks will attempt to lock,
     * then return invalid queues. Locks that are acquired before this method
     * returns will still be valid.
     *
     * Best use is to block while a lock is acquired so that other locks will
     * be blocked. This is useful before terminating the queue guard to avoid
     * other locks to acquire and queue. Method terminate() will clean up the
     * safe queue anyway, but any processing required by the queue items will
     * be delegated to the items' destructors.
     */
    void block();
    /**
     * @brief Resumes a blocked queue to allow further locks.
     * @details This method will counteract the effects of block().
     */
    void unblock();

    SafeQueue<T> lock();
    /**
     * @brief Blocks calling thread until it can acquire the lock for the queue
     * and the queue is empty.
     * @return A wrapper for the queue.
     */
    SafeQueue<T> lockOnEmpty();
    /**
     * @brief Blocks calling thread until it can acquire the lock for the queue
     * and the queue is not empty.
     * @return A wrapper for the queue.
     */
    SafeQueue<T> lockOnNotEmpty();
    /**
     * @brief Blocks calling thread until a push occurs on the queue.
     * @return A wrapper for the queue.
     * @brief If there are more than one thread waiting for a push event,
     * only one will wake up. The rest will continue to wait until they
     * can acquire the lock during a push event.
     */
    SafeQueue<T> lockOnPush();
    /**
     * @brief Blocks calling thread until a pop occurs on the queue or
     * the queue is empty.
     * @return A wrapper for the queue.
     * @brief If there are more than one thread waiting for a pop event,
     * only one will wake up. The rest will continue to wait until they
     * can acquire the lock during a pop event.
     */
    SafeQueue<T> lockOnPop();

    void unlock(SafeQueue<T> &);

    std::size_t size() const;
    bool empty() const;

private:
    void prepareWrapper(SafeQueue<T> &wrapper);

    void onPush();
    void onPop();
    void onEmpty();
    void onNotEmpty();

    std::mutex m_mutex_monitor;
    std::condition_variable m_cv_on_push;
    std::condition_variable m_cv_on_pop;
    std::condition_variable m_cv_on_empty;
    std::condition_variable m_cv_on_not_empty;
    std::condition_variable m_cv_on_entrant_exit;

    bool m_blocked;
    std::size_t m_popped;
    std::size_t m_pushed;
    std::size_t m_entrant_count;

    const std::thread::id m_my_id;
    std::thread::id m_last_lock;

    std::queue<T> *m_queue;
};

} // namespace Threading
} // namespace hebench

#include "inl/safe_queue.inl"

#endif // defined _THREADING_SafeQueue_H_7e5fa8c2415240ea93eff148ed73539b
