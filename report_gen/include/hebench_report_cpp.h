
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d

#include <ratio>
#include <stdexcept>
#include <string>

#include "hebench_report_types.h"

namespace hebench {
namespace ReportGen {
namespace cpp {

//class ReportSummary;

class TimingPrefixUtility
{
public:
    enum class TimeUnit
    {
        Default,
        Seconds,
        MilliSeconds,
        Microseconds,
        Nanoseconds
    };

    static constexpr TimeUnit getPrefix(char ch_prefix)
    {
        switch (ch_prefix)
        {
        case 0:
            return TimeUnit::Default;
            break;
        case 's':
            return TimeUnit::Seconds;
            break;
        case 'm':
            return TimeUnit::MilliSeconds;
            break;
        case 'u':
            return TimeUnit::Microseconds;
            break;
        case 'n':
            return TimeUnit::Nanoseconds;
            break;
        default:
            throw std::invalid_argument("Unknown prefix.");
            break;
        } // end switch
    }

    static constexpr char getPrefix(TimeUnit unit)
    {
        switch (unit)
        {
        case TimeUnit::Default:
            return 0;
            break;
        case TimeUnit::Seconds:
            return 's';
            break;
        case TimeUnit::MilliSeconds:
            return 'm';
            break;
        case TimeUnit::Microseconds:
            return 'u';
            break;
        case TimeUnit::Nanoseconds:
            return 'n';
            break;
        default:
            throw std::invalid_argument("Unknown prefix.");
            break;
        } // end switch
    }

    /**
     * @brief Converts the time in seconds to the specified time unit.
     * @param[out] prefix Structure where to store the result.
     * @param[in] seconds Time in seconds for which to compute prefix.
     * @param[in] ch_prefix Time unit prefix specification. See details.
     * @details
     * Given a timing in seconds and the metric prefix, this function will compute
     * the corresponding value.
     *
     * Values for \p ch_prefix are:
     *
     * 0: behaves as computeTimingPrefix()
     * `'s'`: result is in seconds.
     * `'m'`: result is in milliseconds.
     * `'u'`: result is in microseconds.
     * `'n'`: result is in nanoseconds.
     *
     * Any other value makes the function return a failure.
     *
     * For example, if \p seconds is `0.05` and \p ch_prefix is `m`, then, the result is:
     *
     * @code
     * prefix.value                   = 50;
     * prefix.time_interval_ratio_den = 1000;
     * prefix.symbol                  = 'm';
     * prefix.prefix                  = 'milli';
     * @endcode
     */
    static void setTimingPrefix(TimingPrefixedSeconds &prefix, double seconds, char ch_prefix);
    static void setTimingPrefix(TimingPrefixedSeconds &prefix, double seconds, TimeUnit unit)
    {
        setTimingPrefix(prefix, seconds, getPrefix(unit));
    }
    /**
     * @brief Given a time interval in seconds, computes the timing prefix.
     * @param prefix
     * @param seconds
     * @details The timing prefix is the conversion of the time interval in seconds
     * to the metric scale that best suits the measurement such that the time
     * is in the range `[1, 1000)`. The only prefixes supported are milli, micro,
     * and nano. Thus, the ranges for just seconds is `[0] U [1, infinite)`, and for nano
     * is `(0, 1000)`.
     *
     * For example:
     * @code
     * 12000 seconds <=> 12000 seconds
     * 0.5 seconds <=> 500 milliseconds
     * 0.0005 seconds <=> 500 microseconds
     * 0.0000005 seconds <=> 500 nanoseconds
     * 0.0000000005 seconds <=> 0.5 nanoseconds
     * 0 seconds <=> 0 seconds
     * @endcode
     */
    static void computeTimingPrefix(TimingPrefixedSeconds &prefix, double seconds);
};

class TimingReport
{
    friend class ReportSummary;

private:
    static constexpr const char *m_private_class_name = "TimingReport";

public:
    TimingReport(const TimingReport &) = delete;

public:
    TimingReport(const std::string &header = std::string());
    TimingReport(TimingReport &&);
    virtual ~TimingReport();

    TimingReport &operator=(TimingReport &&);

    void setHeader(const std::string &new_header);
    void appendHeader(const std::string &new_header, bool new_line = true);
    void prependHeader(const std::string &new_header, bool new_line = true);
    std::string getHeader() const;

    void setFooter(const std::string &new_footer);
    void appendFooter(const std::string &new_header, bool new_line = true);
    void prependFooter(const std::string &new_header, bool new_line = true);
    std::string getFooter() const;

    /**
     * @brief Adds a new event type to the types of events.
     * @param event_type_id
     * @param event_type_header
     * @param is_main_event
     * @details
     * event type: groups a collection of events under the same type.
     *
     * If an event of the same type already exists the following will happen:
     *
     * - This method will overwrite the existing event header.
     * - If \p is_main_event is true, the existing event type will be set as the main event.
     * Otherwise, event type status as main event remains unchanged.
     */
    void addEventType(uint32_t event_type_id, const std::string &event_type_header, bool is_main_event = false);
    bool hasEventType(uint32_t event_type_id) const;
    std::string getEventTypeHeader(uint32_t event_type_id) const;
    uint64_t getEventTypeCount() const;
    /**
     * @brief Retrieve an event type ID.
     * @param p_report
     * @param[in] index Index of the event type to retrieve.
     * Must be less than getEventTypeCount() .
     * @return ID of the event type corresponding to the specified index.
     * @exception Instance of std::except on error.
     */
    uint32_t getEventType(uint64_t index) const;
    uint32_t getMainEventType() const;

    // events management

    void addEvent(const TimingReportEventC &p_event);
    void getEvent(TimingReportEventC &p_event, uint64_t index) const;
    uint64_t getEventCount() const;
    uint64_t getEventCapacity() const;
    void setEventCapacity(uint64_t new_capacity);
    void clear();

    // CSV

    void save2CSV(const std::string &filename);
    std::string convert2CSV();

    /**
     * @brief Generates the summary CSV for this report.
     * @param[out] main_event_summary Structure to contain information about the main event
     * for the report. All timings are in seconds.
     * @param[in] time_unit Time unit used throughout the report.
     * @return The summary text that can be directly stored in a CSV file.
     */
    //    std::string generateSummaryCSV(TimingReportEventSummaryC &main_event_summary,
    //                                   TimingPrefixUtility::TimeUnit time_unit = TimingPrefixUtility::TimeUnit::Default);

    static TimingReport loadReportFromCSV(const std::string &s_csv_content);
    static TimingReport loadReportFromCSVFile(const std::string &filename);

    template <class TimeInterval = std::ratio<1, 1>>
    static double computeElapsedWallTime(const TimingReportEventC &event);
    template <class TimeInterval = std::ratio<1, 1>>
    static double computeElapsedCPUTime(const TimingReportEventC &event);

private:
    void *m_lib_handle;
};

template <class TimeInterval>
double TimingReport::computeElapsedWallTime(const TimingReportEventC &event)
{
    return (std::abs(event.wall_time_end - event.wall_time_start)
            * (event.time_interval_ratio_num * TimeInterval::den))
           / (event.time_interval_ratio_den * TimeInterval::num);
}

template <class TimeInterval>
double TimingReport::computeElapsedCPUTime(const TimingReportEventC &event)
{
    return (std::abs(event.cpu_time_end - event.cpu_time_start)
            * (event.time_interval_ratio_num * TimeInterval::den))
           / (event.time_interval_ratio_den * TimeInterval::num);
}

//class ReportSummary
//{
//private:
//    static constexpr const char *m_private_class_name = "ReportSummary";

//public:
//    ReportSummary(const TimingReport &report);
//};

} // namespace cpp
} // namespace ReportGen
} // namespace hebench

#endif // defined _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d
