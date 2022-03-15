
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_MathUtils_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_MathUtils_H_0596d40a3cce4b108a81595c50eb286d

#include <cmath>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <vector>

namespace hebench {
namespace Utilities {
namespace Math {

template <typename T>
/**
 * @brief Finds whether two values are within a certain percentage of each other.
 * @param[in] a First value to compare.
 * @param[in] b Second value to compare.
 * @param[in] pct Per-one for comparison: this is percent divided by 100.
 * @returns true if \p a and \p b are within \p pct * 100 of each other.
 * @returns false otherwise.
 */
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        bool>::type
almostEqual(T a, T b, double pct = 0.05);

template <typename T>
/**
* @brief Finds whether values in two arrays are within a certain percentage of each other.
* @param[in] a Pointer to start of first array to compare.
* @param[in] b Pointer to start of second array to compare.
* @param[in] count Number of elements in each array.
* @param[in] pct Per-one for comparison: this is percent divided by 100.
* @return A vector of `uint64` where each element in this vector is the index of the
* values in \p a and \p b that were not within \p pct * 100 of each other. The return
* vector is empty if all values were within range of each other.
* @details Parameter \p a must hold, at least, \p count elements, same as \p b.
*/
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        std::vector<std::uint64_t>>::type
almostEqual(const T *a, const T *b,
            std::uint64_t count, double pct = 0.05);

/**
 * @brief Provides facilities for a counter by components with limited sizes.
 * @details Given a component counter with sizes (3, 2):
 * @code
 * sizes[0] = 3;
 * sizes[1] = 2;
 * @endcode
 * a ComponentCounter counts from (0, 0) to (2, 1) where each increment looks like:
 * @code
 * (0, 0)
 * (1, 0)
 * (2, 0)
 * (0, 1)
 * (1, 1)
 * (2, 1)
 * @endcode
 */
class ComponentCounter
{
public:
    ComponentCounter(std::vector<std::size_t> component_sizes);

    const std::vector<std::size_t> &getCount() const { return m_count; }
    std::size_t getCountLinear() const;
    std::size_t getCountLinearReversed() const;
    /**
     * @brief Increases the count by 1 on the least significant component
     * (component 0).
     * @returns true if overflow. The counter loops back to 0.
     * @returns false if no overflow occurred.
     */
    bool inc();
    /**
     * @brief Decreases the count by 1 on the least significant component
     * (component 0).
     * @returns true if underflow. The counter loops back to highest count possible.
     * @returns false if no underflow occurred.
     */
    bool dec();
    /**
     * @brief Resets the count to 0.
     */
    void reset();

private:
    std::vector<std::size_t> m_count;
    std::vector<std::size_t> m_sizes;
};

// inline template implementations

template <typename T>
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        bool>::type
//MathUtils::
almostEqual(T a, T b, double pct)
{
    bool retval = a == b;

    if (!retval && pct > 0.0)
    {
        T ab = a * b;
        if (ab > 0)
        {
            a      = std::abs(a);
            b      = std::abs(b);
            pct    = 1.0 - pct;
            retval = (a > b ? b > a * pct : a > b * pct);
        } // end if
        else if (ab < 0)
        {
            T offset = (a < 0 ? a : b);
            a -= offset;
            b -= offset;
            ab = 0;
        } // end if

        if (ab == 0)
            retval = std::abs(a - b) < pct;

    } // end if

    return retval;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        std::vector<std::uint64_t>>::type
//MathUtils::
almostEqual(const T *a, const T *b, std::uint64_t count, double pct)
{
    std::vector<std::uint64_t> retval;
    retval.reserve(count);
    if (a != b)
    {
        if (a && b)
        {
            for (std::uint64_t i = 0; i < count; ++i)
            {
                if (!almostEqual(a[i], b[i], pct))
                    retval.push_back(i);
            } // end for
        } // end if
        else
        {
            // all elements differ if one is null
            retval.resize(count);
            std::iota(retval.begin(), retval.end(), 0UL);
        } // end else
    } // end if
    return retval;
}

} // namespace Math
} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Harness_MathUtils_H_0596d40a3cce4b108a81595c50eb286d
