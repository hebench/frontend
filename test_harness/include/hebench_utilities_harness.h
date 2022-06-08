
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d

#include <functional>
#include <ostream>
#include <random>
#include <string>

#include "hebench/api_bridge/types.h"
#include "hebench_report_cpp.h"
#include "modules/timer/include/timer.h"

namespace hebench {
namespace Utilities {

/**
 * @brief Converts a string to directory name friendly.
 * @param[in] s String to convert.
 * @param[in] to_lowercase Specifies whether to return all lowercase.
 * @return A string that can be used as a directory name.
 * @details This function will return a string generated from \p s where the result
 * is composed only of alpha-numeric characters and non-repeating underscores.
 * The result will have no leading or trailing underscores. If \p to_lowercase flag
 * is true, all characters will lowercase, otherwise, they will keep the case originally
 * encountered in \p s. Non-alpha-numeric characters will be turned into underscores.
 *
 * For example:
 * @code
 * s = "Hello, World!"
 *
 * std::cout << convertToDirectoryName(s, true);
 *
 * // Prints:
 * // hello_world
 * @endcode
 */
std::string convertToDirectoryName(const std::string &s, bool to_lowercase = true);

/**
 * @brief Writes the collection of `NativeDataBuffer` as columns to the specified
 * output stream.
 * @param os Stream where to write.
 * @param[in] p_buffers Array of `NativeDataBuffer` where each element will be output
 * per column. Each element does not need to have the same number of rows.
 * @param[in] count Number of elements in array \p p_buffers.
 * @param[in] data_type Type of data pointed to by \p p_buffers ' `NativeDataBuffer`.
 * @param[in] separator Separator text to use when separating the columns.
 * @details The number of elements in each buffer pointed by \p p_buffers does not
 * need to match. This method will print as many elements as each buffer has per row,
 * so buffers with fewer elements will be skipped in the output after all their elements
 * have been printed out.
 */
void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          hebench::APIBridge::DataType data_type,
                          bool output_row_index = false,
                          const char *separator = " ");
template <typename T>
/**
 * @brief Writes the collection of `NativeDataBuffer` as columns to the specified
 * output stream.
 * @param os Stream where to write.
 * @param[in] p_buffers Array of `NativeDataBuffer` where each element will be output
 * per column. Each element does not need to have the same number of rows. It is assumed
 * that buffers in this array point to data of type `T`.
 * @param[in] count Number of elements in array \p p_buffers.
 * @param[in] separator Separator text to use when separating the columns.
 * @details The number of elements in each buffer pointed by \p p_buffers does not
 * need to match. This method will print as many elements as each buffer has per row,
 * so buffers with fewer elements will be skipped in the output after all their elements
 * have been printed out.
 */
void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          bool output_row_index = false,
                          const char *separator = " ");

class RandomGenerator
{
private:
    static std::mt19937 m_rand;

public:
    static decltype(m_rand) &get() { return m_rand; }
    static void setRandomSeed(std::uint64_t seed);
    static void setRandomSeed();
};

class TimingReportEx : public hebench::ReportGen::cpp::TimingReport
{
private:
    using Base = hebench::ReportGen::cpp::TimingReport;

public:
    TimingReportEx(const std::string &header = std::string()) :
        TimingReport(header) {}
    ~TimingReportEx() override {}

    using Base::addEvent;
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event);
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                  const std::string &event_type_name);

    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    static hebench::ReportGen::TimingReportEventC convert2C(const hebench::Common::TimingReportEvent &timing_event);

private:
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                  const char *event_type_name);
};

} // namespace Utilities
} // namespace hebench

#include "inl/hebench_utilities_harness.inl"

#endif // defined _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d
