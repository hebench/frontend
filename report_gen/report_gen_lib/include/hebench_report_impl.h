
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_TimingReport_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_TimingReport_H_0596d40a3cce4b108a81595c50eb286d

#include <cmath>
#include <iostream>
#include <memory>
#include <ratio>
#include <string>
#include <unordered_map>
#include <vector>

#include "hebench_report.h"

namespace hebench {
namespace TestHarness {
namespace Report {

class TimingReport
{
public:
    // section indicators for parsing generated CSV

    static constexpr const char *TagVersion      = "#v,0,1,1";
    static constexpr const char *TagReportHeader = "#0100"; // header at the start of the test
    static constexpr const char *TagFailedTest   = "#XXXX"; // indicates failed test (validation failed)
    static constexpr const char *TagReportData   = "#0200"; // start of the data
    static constexpr const char *TagReportFooter = "#8E00"; // start of the footer
    static constexpr const char *TagReportEnd    = "#8FFF"; // end of the report

    template <class TimeInterval = std::ratio<1, 1>>
    static double computeElapsedWallTime(const TimingReportEventC &event);
    template <class TimeInterval = std::ratio<1, 1>>
    static double computeElapsedCPUTime(const TimingReportEventC &event);

    TimingReport();

    void newEventType(std::uint32_t set_id, const std::string &set_header, bool is_main_event = false);
    const std::unordered_map<std::uint32_t, std::string> &getEventTypes() const { return m_event_headers; }

    std::uint32_t getMainEventID() const { return m_main_event; }

    void newEvent(std::shared_ptr<TimingReportEventC> p_event, const std::string &set_header = std::string());
    const std::vector<std::shared_ptr<TimingReportEventC>> &getEvents() const { return m_events; }
    void reserveCapacityForEvents(std::size_t new_capacity);
    void clear();

    const std::string &getHeader() const { return m_header; }
    void setHeader(const std::string &header) { m_header = header; }
    void appendHeader(const std::string &header, bool new_line);
    void prependHeader(const std::string &header, bool new_line);

    const std::string &getFooter() const { return m_footer; }
    void setFooter(const std::string &footer) { m_footer = footer; }
    void appendFooter(const std::string &footer, bool new_line);
    void prependFooter(const std::string &footer, bool new_line);

    std::ostream &convert2CSV(std::ostream &os) const;

    static TimingReport loadCSV(std::istream &is);
    static TimingReport loadCSV(const std::string &csv_content);

    static void computeTimingPrefix(TimingPrefixedSeconds &prefix, double seconds);

private:
    /**
     * @brief Given a CSV line, returns the next value and its bounds in the line.
     * @param[out] out_start_idx
     * @param[out] out_count
     * @param[in] s CSV line null-terminated string.
     * @return std::string_view(\p s + \p out_start_idx, \p out_count)
     * @details Given a line:
     * @code
     * value0,value1, ...
     * @endcode
     * returns value0, and the start index and number of characters of value0 inside \p s.
     */
    static std::string_view findNextValueCSV(std::size_t &out_start_idx,
                                             std::size_t &out_count,
                                             const char *s);
    /**
     * @brief Parses the immidate heading and value from the specified CSV line.
     * @param s_out_heading
     * @param out_value
     * @param s_line
     * @details Expected line format:
     * @code
     * <heading: string>,<value:uint64_t>[, ...]
     * @endcode
     */
    static void parseHeadingValue(std::string &s_out_heading, std::uint64_t &out_value,
                                  const std::string &s_line);
    /**
     * @brief Parses the immidate heading and value from the specified CSV line.
     * @param s_out_heading
     * @param out_value
     * @param s_line
     * @details Expected line format:
     * @code
     * <heading: string>,<value:double>[, ...]
     * @endcode
     */
    static void parseHeadingValue(std::string &s_out_heading, double &out_value,
                                  const std::string &s_line);
    /**
     * @brief Retrieve next line from the stream and trim leading and trailing whitespaces
     * and extra characters.
     * @param is
     * @param[out] s
     * @param[in] extra_trim If not null, specifies a collection of extra characters to trim
     * off the read line.
     * @return
     */
    static std::istream &getTrimmedLine(std::istream &is, std::string &s_out,
                                        const std::string &extra_trim = std::string());
    /**
     * @brief Retrieve next line from the stream and trim leading and trailing whitespaces
     * and extra characters.
     * @param is
     * @param[out] s
     * @param[in] extra_ltrim If not null, specifies a collection of extra characters to trim
     * off the read line from the left.
     * @param[in] extra_rtrim If not null, specifies a collection of extra characters to trim
     * off the read line from the right.
     * @return
     */
    static std::istream &getTrimmedLine(std::istream &is, std::string &s_out,
                                        const std::string &extra_ltrim, const std::string &extra_rtrim);
    /**
     * @brief Reads a block of text from the current position of the
     * input stream until the specified tag (or end of stream) is found.
     * @param is
     * @param[out] s_block Reference to string where to store the read block of text.
     * @param[in] tags Array of tags to look for.
     * @return Tag that was found or empty if end of stream reached.
     */
    static std::string readTextBlock(std::istream &is, std::string &s_block,
                                     const std::vector<std::string> tags);
    static void parseTimingEvent(std::string &s_out_event_header,
                                 std::shared_ptr<TimingReportEventC> &p_out_event,
                                 const std::string &s_line);

    std::string m_header;
    std::string m_footer;
    std::uint32_t m_main_event;
    std::unordered_map<std::uint32_t, std::string> m_event_headers; // maps event id to event header
    std::vector<std::shared_ptr<TimingReportEventC>> m_events;
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

class ReportSummary
{
public:
    ReportSummary(const TimingReport &report);

    const std::string &getHeader() const { return m_header; }
    const std::string &getFooter() const { return m_footer; }

    std::uint64_t getEventSummaryCount() const { return m_event_summaries.size(); }
    const TimingReportEventSummaryC &getEventSummary(std::uint64_t index) const;
    std::uint64_t getMainEventSummaryIndex() const { return m_main_event_index; }
    const TimingReportEventSummaryC &getMainEventSummary() const;

    void generateCSV(std::ostream &os);

private:
    std::string m_header;
    std::string m_footer;
    std::size_t m_main_event_index;
    std::vector<std::shared_ptr<TimingReportEventSummaryC>> m_event_summaries;
};

} // namespace Report
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_TimingReport_H_0596d40a3cce4b108a81595c50eb286d
