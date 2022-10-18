
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Common_MathUtils_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Common_MathUtils_H_0596d40a3cce4b108a81595c50eb286d

#include <cmath>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <vector>

namespace hebench {
namespace Utilities {
namespace Math {

/**
 * @brief Maintains a running mean and variance of a collection of events.
 */
class EventStats
{
public:
    EventStats() :
        m_count(0), m_total(0.0), m_mean(0.0), m_m2(0.0),
        m_min(std::numeric_limits<double>::infinity()),
        m_max(-std::numeric_limits<double>::infinity())
    {
    }

    void newEvent(double x)
    {
        ++m_count;
        m_total += x;
        double d = x - m_mean;
        m_mean += d / m_count;
        double d2 = x - m_mean;

        m_m2 = m_m2 + d * d2;

        if (m_min > x)
            m_min = x;
        if (m_max < x)
            m_max = x;
    }

    /**
     * @brief Total of all events.
     * @return Sum of all event values.
     * @details Note that this value may overflow. To avoid unstable
     * values as much as possible, mean and variance are computed using
     * running mean and variance algorithms instead of this total.
     */
    double getTotal() const { return m_total; }
    std::size_t getCount() const { return m_count; }
    double getMin() const { return m_min; }
    double getMax() const { return m_max; }
    double getMean() const { return m_mean; }
    double getVariance() const
    {
        if (m_count < 2)
            return 0;
        else
            return m_m2 / (m_count - 1);
    }

private:
    std::size_t m_count;
    double m_total;
    double m_mean;
    double m_m2;
    double m_min;
    double m_max;
};

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
 * @brief Computes the percentile value on given pre-sorted data, and a normalized percentile (0 to 1 inclussive).
 * @param[in] data Points to presorted data.
 * @param[in] count Number of data points pointed to by \p data .
 * @param[in] percentile Normalized percentile (0 to 1 inclussive).
 * @return The percentile value on the specified dataset corresponding to the given percentile.
 * @details This function uses the same formula for percentile calculation as R.
 */
double computePercentile(const double *data, std::size_t count, double percentile);

// inline template implementations

template <typename T>
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        bool>::type
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

#endif // defined _HEBench_Common_MathUtils_H_0596d40a3cce4b108a81595c50eb286d
