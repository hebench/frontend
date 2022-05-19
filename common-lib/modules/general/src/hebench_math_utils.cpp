
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <stdexcept>

#include "../include/hebench_math_utils.h"

namespace hebench {
namespace Utilities {
namespace Math {

double computePercentile(const double *data, std::size_t count, double percentile)
{
    // uses percentile formula from R
    double rank          = std::clamp(percentile, 0.0, 1.0) * (count - 1);
    std::uint64_t rank_i = static_cast<std::uint64_t>(rank);
    double rank_f        = rank - rank_i;

    return data[rank_i] + rank_f * (data[rank_i + 1] - data[rank_i]);
}

//------------------------
// class ComponentCounter
//------------------------

ComponentCounter::ComponentCounter(std::vector<std::size_t> component_sizes) :
    m_count(component_sizes.size()), m_sizes(component_sizes)
{
    for (std::size_t i = 0; i < component_sizes.size(); ++i)
        if (component_sizes[i] <= 0)
            throw std::invalid_argument("Invalid zero entry: 'component_sizes'.");
}

std::size_t ComponentCounter::getCountLinear() const
{
    std::size_t retval = m_count.empty() ? 0 : m_count.back();
    for (std::size_t i = m_count.size() - 1; i > 0; --i)
        retval = m_count[i - 1] + m_sizes[i - 1] * retval;
    return retval;
}

std::size_t ComponentCounter::getCountLinearReversed() const
{
    std::size_t retval = m_count.empty() ? 0 : m_count[0];
    for (std::size_t i = 1; i < m_count.size(); ++i)
        retval = m_count[i] + m_sizes[i] * retval;
    return retval;
}

bool ComponentCounter::inc()
{
    bool overflow = true;
    for (std::size_t i = 0; overflow && i < m_count.size(); ++i)
    {
        if (++m_count[i] >= m_sizes[i])
            m_count[i] = 0;
        else
            overflow = false;
    } // end for

    return overflow;
}

bool ComponentCounter::dec()
{
    bool overflow = true;
    for (std::size_t i = 0; overflow && i < m_count.size(); ++i)
    {
        if (m_count[i] == 0)
            m_count[i] = m_sizes[i] - 1;
        else
        {
            --m_count[i];
            overflow = false;
        } // end else
    } // end for

    return overflow;
}

void ComponentCounter::reset()
{
    std::fill(m_count.begin(), m_count.end(), 0);
}

} // namespace Math
} // namespace Utilities
} // namespace hebench
