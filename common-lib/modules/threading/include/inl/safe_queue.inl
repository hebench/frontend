// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _THREADING_SafeQueue_SRC_7e5fa8c2415240ea93eff148ed73539b
#define _THREADING_SafeQueue_SRC_7e5fa8c2415240ea93eff148ed73539b

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "../safe_queue.h"

namespace hebench {
namespace Threading {

template <class T>
inline SafeQueue<T>::SafeQueue(std::mutex &mutex) :
    m_mutex(&mutex), m_monitor(mutex), m_pq(nullptr), m_powner(nullptr)
{
}

template <class T>
inline SafeQueue<T>::~SafeQueue()
{
    if (m_powner)
        m_powner->unlock(*this);
}

template <class T>
inline SafeQueue<T>::SafeQueue(SafeQueue &&src)
{
    m_mutex   = src.m_mutex;
    m_monitor = std::move(src.m_monitor);
    prepare(std::this_thread::get_id(), src.m_pq, src.m_powner, src.m_on_push, src.m_on_pop, src.m_on_empty, src.m_on_not_empty);
    src.terminate();
}

template <class T>
inline SafeQueue<T> &SafeQueue<T>::operator=(SafeQueue &&src)
{
    if (this != &src)
    {
        m_mutex   = src.m_mutex;
        m_monitor = std::move(src.m_monitor);

        prepare(std::this_thread::get_id(), src.m_pq, src.m_powner, src.m_on_push, src.m_on_pop, src.m_on_empty, src.m_on_not_empty);
        src.terminate();
    }
    return *this;
}

template <class T>
inline void SafeQueue<T>::validate(const char *func) const
{
    if (!this->valid())
    {
        std::stringstream ss;
        if (func)
            ss << func << ": ";
        ss << "Invalid operation on SafeQueue: ";
        if (!m_pq)
            ss << "object disposed.";
        else if (std::this_thread::get_id() != m_th_id)
            ss << "thread safety violation.";
        else
            ss << "object in invalid state.";
        throw std::logic_error(ss.str());
    }
}

template <class T>
inline void SafeQueue<T>::prepare(const std::thread::id &owner_th, std::queue<T> *queue, SafeQueueGuard<T> *owner,
                                  std::function<void()> on_push, std::function<void()> on_pop, std::function<void()> on_empty,
                                  std::function<void()> on_not_empty)
{
    m_th_id = owner_th;

    m_powner       = owner;
    m_pq           = queue;
    m_on_push      = on_push;
    m_on_pop       = on_pop;
    m_on_empty     = on_empty;
    m_on_not_empty = on_not_empty;
}

template <class T>
inline void SafeQueue<T>::terminate()
{
    m_powner = nullptr;
    m_pq     = nullptr;

    if (m_monitor.owns_lock())
        m_monitor.unlock();
}

template <class T>
inline bool SafeQueue<T>::empty() const
{
    validate(__func__);
    return m_pq->empty();
}

template <class T>
inline std::size_t SafeQueue<T>::size() const
{
    validate(__func__);
    return m_pq->size();
}

template <class T>
inline typename std::queue<T>::reference SafeQueue<T>::front()
{
    validate(__func__);
    return m_pq->front();
}

template <class T>
inline typename std::queue<T>::const_reference SafeQueue<T>::front() const
{
    validate(__func__);
    return m_pq->front();
}

template <class T>
inline typename std::queue<T>::reference SafeQueue<T>::back()
{
    validate(__func__);
    return m_pq->back();
}

template <class T>
inline typename std::queue<T>::const_reference SafeQueue<T>::back() const
{
    validate(__func__);
    return m_pq->back();
}

template <class T>
inline void SafeQueue<T>::push(const typename std::queue<T>::value_type &val)
{
    validate(__func__);

    bool empty_state = m_pq->empty();

    m_pq->push(val);

    if (empty_state && m_on_not_empty)
        m_on_not_empty();

    if (m_on_push)
        m_on_push();
}

template <class T>
inline void SafeQueue<T>::push(typename std::queue<T>::value_type &&val)
{
    validate(__func__);

    bool empty_state = m_pq->empty();

    m_pq->push(val);

    if (empty_state && m_on_not_empty)
        m_on_not_empty();

    if (m_on_push)
        m_on_push();
}

template <class T>
inline void SafeQueue<T>::pop()
{
    validate(__func__);

    m_pq->pop();

    if (m_on_pop)
        m_on_pop();

    if (m_pq->empty() && m_on_empty)
        m_on_empty();
}

template <class T>
inline void SafeQueue<T>::swap(SafeQueue &x)
{
    validate(__func__);

    m_pq->swap(*x.m_pq);

    if (m_pq->empty() && m_on_empty)
        m_on_empty();
}

template <class T>
inline bool SafeQueue<T>::valid() const
{
    return std::this_thread::get_id() == m_th_id && m_pq != nullptr;
}

//------------------------------------
// class SafeQueueGuard
//------------------------------------

template <class T>
inline SafeQueueGuard<T>::SafeQueueGuard() :
    m_blocked(false), m_popped(0), m_pushed(0), m_entrant_count(0), m_my_id(std::this_thread::get_id())
{
    m_queue = new std::queue<T>();
}

template <class T>
inline SafeQueueGuard<T>::~SafeQueueGuard()
{
    try
    {
        terminate();
    }
    catch (...)
    {
    }
}

template <class T>
inline void SafeQueueGuard<T>::terminate()
{
    std::unique_lock<std::mutex> lock_cs(m_mutex_monitor);
    if (m_queue)
    {
        if (std::this_thread::get_id() != m_my_id)
            throw std::logic_error("SafeQueueGuard objects can only be terminated on the same thread that created them.");
        if (m_my_id == m_last_lock)
            throw std::logic_error("Deadlock detected: SafeQueue is locked by same thread terminating SafeQueueGuard.");

        // terminate queue

        delete m_queue;
        m_queue = nullptr;

        // notify all waiting threads to stop

        m_cv_on_push.notify_all();
        m_cv_on_pop.notify_all();
        m_cv_on_empty.notify_all();
        m_cv_on_not_empty.notify_all();

        // wait until all threads waiting to lock complete
        while (m_entrant_count > 0)
            m_cv_on_entrant_exit.wait(lock_cs);
    }
}

template <class T>
inline void SafeQueueGuard<T>::block()
{
    m_blocked = true;
}

template <class T>
inline void SafeQueueGuard<T>::unblock()
{
    m_blocked = false;
}

template <class T>
inline SafeQueue<T> SafeQueueGuard<T>::lock()
{
    SafeQueue<T> retval(m_mutex_monitor);
    m_entrant_count++; // entering lock function

    m_last_lock = std::this_thread::get_id();
    prepareWrapper(retval);

    m_entrant_count--; // exiting lock function
    m_cv_on_entrant_exit.notify_all();

    if (!retval.valid())
        unlock(retval);

    return retval;
}

template <class T>
inline SafeQueue<T> SafeQueueGuard<T>::lockOnEmpty()
{
    SafeQueue<T> retval(m_mutex_monitor);
    m_entrant_count++; // entering lock function

    while (m_queue && !m_queue->empty())
        m_cv_on_empty.wait(retval.m_monitor);
    m_last_lock = std::this_thread::get_id();

    prepareWrapper(retval);

    m_entrant_count--; // exiting lock function
    m_cv_on_entrant_exit.notify_all();

    if (!retval.valid())
        unlock(retval);

    return retval;
}

template <class T>
inline SafeQueue<T> SafeQueueGuard<T>::lockOnNotEmpty()
{
    SafeQueue<T> retval(m_mutex_monitor);
    m_entrant_count++; // entering lock function

    while (m_queue && m_queue->empty())
        m_cv_on_not_empty.wait(retval.m_monitor);
    m_last_lock = std::this_thread::get_id();

    prepareWrapper(retval);

    m_entrant_count--; // exiting lock function
    m_cv_on_entrant_exit.notify_all();

    if (!retval.valid())
        unlock(retval);

    return retval;
}

template <class T>
inline SafeQueue<T> SafeQueueGuard<T>::lockOnPush()
{
    SafeQueue<T> retval(m_mutex_monitor);
    m_entrant_count++; // entering lock function

    while (m_queue && m_pushed <= 0)
        // wait for push event
        m_cv_on_push.wait(retval.m_monitor);
    m_last_lock = std::this_thread::get_id();
    if (m_pushed > 0)
        m_pushed--; // consumed

    prepareWrapper(retval);

    m_entrant_count--; // exiting lock function
    m_cv_on_entrant_exit.notify_all();

    if (!retval.valid())
        unlock(retval);

    return retval;
}

template <class T>
inline SafeQueue<T> SafeQueueGuard<T>::lockOnPop()
{
    SafeQueue<T> retval(m_mutex_monitor);
    m_entrant_count++; // entering lock function

    while (m_queue && m_popped <= 0 && !m_queue->empty())
        // wait for pop event or empty queue
        m_cv_on_pop.wait(retval.m_monitor);
    m_last_lock = std::this_thread::get_id();
    if (m_popped > 0)
        m_popped--; // consumed

    prepareWrapper(retval);

    m_entrant_count--; // exiting lock function
    m_cv_on_entrant_exit.notify_all();

    if (!retval.valid())
        unlock(retval);

    return retval;
}

template <class T>
inline void SafeQueueGuard<T>::unlock(SafeQueue<T> &w)
{
    if (m_queue != w.m_pq)
        throw std::runtime_error("Invalid unlock token.");
    m_last_lock = std::thread::id(); // no current lock
    w.terminate();
}

template <class T>
inline std::size_t SafeQueueGuard<T>::size() const
{
    std::lock_guard<std::mutex> monitor(m_mutex_monitor);
    if (!m_queue)
        throw std::logic_error("Invalid operation on disposed SafeQueue: size");
    return m_queue->size();
}

template <class T>
inline bool SafeQueueGuard<T>::empty() const
{
    std::lock_guard<std::mutex> monitor(m_mutex_monitor);
    if (!m_queue)
        throw std::logic_error("Invalid operation on disposed SafeQueue: empty");
    return m_queue->empty();
}

template <class T>
inline void SafeQueueGuard<T>::onPush()
{
    // this method will always be called from a single thread because
    // it is only visible by a safe queue that has already been locked
    m_pushed++;
    m_cv_on_push.notify_all();
}

template <class T>
inline void SafeQueueGuard<T>::onPop()
{
    // this method will always be called from a single thread because
    // it is only visible by a safe queue that has already been locked
    m_popped++;
    m_cv_on_pop.notify_all();
}

template <class T>
inline void SafeQueueGuard<T>::onEmpty()
{
    // this method will always be called from a single thread because
    // it is only visible by a safe queue that has already been locked
    m_cv_on_empty.notify_all();
}

template <class T>
inline void SafeQueueGuard<T>::onNotEmpty()
{
    // this method will always be called from a single thread because
    // it is only visible by a safe queue that has already been locked
    m_cv_on_not_empty.notify_all();
}

template <class T>
inline void SafeQueueGuard<T>::prepareWrapper(SafeQueue<T> &wrapper)
{
    if (!m_blocked)
        wrapper.prepare(
            std::this_thread::get_id(), m_queue, this, [this]() { this->onPush(); }, [this]() { this->onPop(); },
            [this]() { this->onEmpty(); }, [this]() { this->onNotEmpty(); });
}

} // namespace Threading
} // namespace hebench

#endif // defined _THREADING_SafeQueue_SRC_7e5fa8c2415240ea93eff148ed73539b
