
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Report_Types_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Report_Types_H_0596d40a3cce4b108a81595c50eb286d

#include <stdint.h>

namespace hebench {
namespace TestHarness {
namespace Report {

extern "C"
{

#define MAX_DESCRIPTION_BUFFER_SIZE            256
#define MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE 256

    struct _TimingReportEventC
    {
        /**
         * @brief ID specifying the event type.
         * @details During summary report, events with the same ID will be
         * grouped together for statistical computations. Users must make sure
         * that related events are tagged with the correct ID.
         */
        uint32_t event_type_id;
        double cpu_time_start;
        double cpu_time_end;
        double wall_time_start;
        double wall_time_end;
        /**
         * @brief Scale of time interval used for this event.
         * @details Timing values in this instance are given in this scale.
         *
         * This represents the ratio of this scale with respect to `1` second.
         * For example, `1` millisecond is `0.001` seconds, and thus, its
         * scale ratio is `1:1000`. So, the time, in seconds for wall time is
         * @code
         * (wall_time_end - wall_time_start) * time_interval_ratio_num / time_interval_ratio_den
         * @endcode
         * Similarly for CPU time.
         */
        int64_t time_interval_ratio_num;
        /**
         * @brief Scale of time interval used for this event.
         * @sa time_interval_ratio_num
         */
        int64_t time_interval_ratio_den;
        uint64_t iterations;
        /**
         * @brief Description attached to this event.
         * @details Set to empty string if no description.
         */
        char description[MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE];
    };
    typedef struct _TimingReportEventC TimingReportEventC;

    struct _TimingReportEventSummaryC
    {
        /**
         * @brief ID specifying the event type.
         * @details During summary report, events with the same ID are
         * grouped together for statistical computations. Users must make sure
         * that related events are tagged with the correct ID.
         */
        uint32_t event_id;
        double cpu_time_ave;
        double cpu_time_variance;
        double wall_time_ave;
        double wall_time_variance;
        uint64_t iterations;
        /**
         * @brief Event name.
         */
        char name[MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE];
    };
    typedef struct _TimingReportEventSummaryC TimingReportEventSummaryC;

#define MAX_SYMBOL_BUFFER_SIZE 4
    struct _UnitPrefix
    {
        /**
         * @brief Value in the specified unit.
         */
        double value;
        /**
         * @brief Denominator of timing scale ratio with respect to a unit.
         * @details Numerator is always 1.
         */
        int64_t time_interval_ratio_den;
        /**
         * @brief Symbol for the prefix.
         * @details m for milli, u micro, n for nano
         */
        char symbol[MAX_SYMBOL_BUFFER_SIZE];
        /**
         * @brief Full name for the prefix (or empty string if no prefix).
         * @details milli, micro, nano
         */
        char prefix[MAX_DESCRIPTION_BUFFER_SIZE];
    };
    typedef struct _UnitPrefix TimingPrefixedSeconds;
}

} // namespace Report
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Report_Types_H_0596d40a3cce4b108a81595c50eb286d
