// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __COMMON_Pool_H_7e5fa8c2415240ea93eff148ed73539b
#define __COMMON_Pool_H_7e5fa8c2415240ea93eff148ed73539b

#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>

#include "nocopy.h"

namespace hebench {
namespace Common {

/**
 * Encapsulates a simple pool of resource objects of the same type.
 * Objects must to be copyable.
 */
template <class T>
class SimplePool
{
public:
    DISABLE_COPY(SimplePool)

    typedef std::shared_ptr<SimplePool<T>> Ptr;

    /**
     * @brief Constructs an empty pool.
     */
    SimplePool() {}
    /**
     * @brief Constructs pool owning the elements of the specified vector.
     * @param arr[in] Vector containing the elements to add to this pool.
     * Duplicate elements will be ignored.
     */
    SimplePool(const std::vector<T> &arr)
    {
        for (std::size_t i = 0; i < arr.size(); i++)
            AddElement(arr[i]);
    }
    /**
     * @brief Destroys a SimplePool object.
     * @details The destructor of all owned elements will be called.
     * All owned elements are copies of their original values.
     */
    virtual ~SimplePool() {}

    /**
     * @brief Adds a new element to the elements owned by the pool.
     * The element becomes available if it is not already part of the pool.
     * @param value[in] Element to add to the pool. If the element already
     * exists it will not be duplicated.
     */
    virtual void addElement(const T &value)
    {
        if (m_total.insert(value).second)
            // element inserted; it is new to the pool
            m_available.push(value);
    }

    /**
     * @brief Allows access to the collection of all the elements owned by the
     * pool.
     * @return A reference to an unordered_set of all the elements owned by the
     * pool.
     */
    const std::unordered_set<T> &elements() const noexcept { return m_total; }

    /**
     * @brief Returns the number of elements owned by the pool.
     * @return The number of elements owned by the pool.
     */
    std::size_t count() const noexcept { return m_total.size(); }

    /**
     * @brief Returns the number of available elements in the pool.
     * @return The number of available elements in the pool.
     */
    virtual std::size_t availableCount() const { return m_available.size(); }

    /**
     * @brief Empties the pool. After this call, the pool is in
     * a state as if default constructed.
     * @details Elements that have been popped from the pool will
     * become orphaned, this is, they cannot be pushed back into
     * the pool.
     *
     * The destructor of all owned elements still in the pool
     * will be called. All owned elements are copies of their
     * original values.
     */
    virtual void clear()
    {
        // clear queue
        {
            std::queue<T> empty;
            std::swap(m_available, empty);
        }
        // clear set
        m_total.clear();
    }

    /**
     * @brief Removes and returns the first available element from the pool.
     * @return The first available element from the pool. Return value is
     * undefined if any error occurred.
     * @throw std::logic_error: There are no elements available in the pool.
     */
    virtual T pop()
    {
        T retval;

        if (m_available.empty())
            throw std::logic_error("Out of resources.");
        else
        {
            retval = m_available.front();
            m_available.pop();
        }
        return retval;
    }

    /**
     * @brief Restores an element to the pool.
     * @param value[in] Element to return to the pool. It must be part of the
     * Elements owned by this pool.
     * @throw std::invalid_argument: Trying to restore element to the wrong
     * pool.
     */
    virtual void push(const T &value)
    {
        // check if element belongs to this pool
        if (m_total.count(value) == 0)
            throw std::invalid_argument("value: element does not belong to this pool.");
        else
            m_available.push(value);
    }

private:
    std::unordered_set<T> m_total; // all elements owned by the pool container
    std::queue<T> m_available; // all available elements in the pool
};

} // namespace Common
} // namespace hebench

#endif // defined __COMMON_Pool_H_7e5fa8c2415240ea93eff148ed73539b
