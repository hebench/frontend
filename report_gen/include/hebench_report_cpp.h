
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d

#include <string>

#include "hebench_report_types.h"

namespace hebench {
namespace TestHarness {
namespace Report {
namespace cpp {

class TimingReport
{
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

    // event type: groups a collection of events under the same type

    /**
     * @brief Adds a new event type to the types of events.
     * @param event_type_id
     * @param event_type_header
     * @param is_main_event
     * @details
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

    std::string generateSummaryCSV(TimingReportEventC &main_event_summary);

    static TimingReport loadReportFromCSV(const std::string &s_csv_content);
    static TimingReport loadReportFromCSVFile(const std::string &filename);

    // utils

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

private:
    void *m_lib_handle;
};

} // namespace cpp
} // namespace Report
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Report_CPP_H_0596d40a3cce4b108a81595c50eb286d
