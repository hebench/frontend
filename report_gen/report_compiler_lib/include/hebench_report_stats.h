
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_ReportStats_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_ReportStats_H_0596d40a3cce4b108a81595c50eb286d

#include <cmath>
#include <iostream>
#include <memory>
#include <ratio>
#include <string>
#include <unordered_map>
#include <vector>

#include "hebench_report_cpp.h"
#include "hebench_report_types.h"

namespace hebench {
namespace ReportGen {

struct StatisticsResult
{
    double total; // total wall time for the all the events
    double ave;
    double variance;
    double min;
    double max;
    double median;
    double pct_1; // 1-th percentile
    double pct_10; // 10-th percentile
    double pct_90; // 90-th percentile
    double pct_99; // 99-th percentile
    double ave_trim; // trimmed by 10% on each side
    double variance_trim;
    double samples_per_unit; // = total / input_sample_count
    double samples_per_unit_trim; // = total_trim / input_sample_count_trim
    uint64_t input_sample_count;
};

struct ReportEventTypeStats
{
    /**
     * @brief ID specifying the event type.
     * @details During summary report, events with the same ID are
     * grouped together for statistical computations. Users must make sure
     * that related events are tagged with the correct ID.
     */
    uint32_t event_id;
    double total_time; // total wall time for the all the events
    double cpu_time_ave;
    double cpu_time_variance;
    double cpu_time_min;
    double cpu_time_max;
    double cpu_time_median;
    double cpu_time_1; // 1-th percentile
    double cpu_time_10; // 10-th percentile
    double cpu_time_90; // 90-th percentile
    double cpu_time_99; // 99-th percentile
    double cpu_time_ave_trim; // trimmed mean by 10% on each side
    double cpu_time_variance_trim;
    double wall_time_ave;
    double wall_time_variance;
    double wall_time_min;
    double wall_time_max;
    double wall_time_median;
    double wall_time_1; // 1-th percentile
    double wall_time_10; // 10-th percentile
    double wall_time_90; // 90-th percentile
    double wall_time_99; // 99-th percentile
    double wall_time_ave_trim; // trimmed mean by 10% on each side
    double wall_time_variance_trim;
    double ops_per_sec;
    double ops_per_sec_trim; // ops/sec based on trimmed mean
    /**
     * @brief Number of input samples used.
     * @details This is, if the operation were a latency operation,
     * it was executed once for each input sample.
     */
    uint64_t input_sample_count;
    /**
     * @brief Event name.
     */
    std::string name;
    std::string description;
};

class ReportStats
{
public:
    ReportStats(const cpp::TimingReport &report);

    const std::string &getHeader() const { return m_header; }
    const std::string &getFooter() const { return m_footer; }

    std::uint64_t getEventTypeCount() const { return m_event_stats.size(); }
    const ReportEventTypeStats &getEventTypeStats(std::uint64_t index) const;
    const ReportEventTypeStats &getEventTypeStatsByID(std::uint32_t id) const;
    std::uint64_t getMainEventTypeStatsIndex() const;
    std::uint32_t getMainEventTypeID() const { return m_main_event_type_id; }
    const ReportEventTypeStats &getMainEventTypeStats() const;

    /**
     * @brief Generates complete CSV stats for this report.
     * @param os
     * @param ch_prefix
     */
    void generateCSV(std::ostream &os, char ch_prefix);
    /**
     * @brief Generates a row of CSV for stats for a single event type.
     * @param os
     * @param index
     * @param ch_prefix
     * @param[in] new_line If true, a new line is added at the end of the row.
     */
    void generateCSV(std::ostream &os, const ReportEventTypeStats &stats, char ch_prefix, bool new_line = true);
    /**
     * @brief Generates summary CSV for this report.
     * @param os
     * @param ch_prefix
     */
    void generateSummaryCSV(std::ostream &os, char ch_prefix);
    /**
     * @brief Generates a row of CSV summary for a single event type.
     * @param os
     * @param stats
     * @param ch_prefix
     * @param[in] new_line If true, a new line is added at the end of the row.
     */
    void generateSummaryCSV(std::ostream &os, const ReportEventTypeStats &stats, char ch_prefix, bool new_line = true);

private:
    std::string m_header;
    std::string m_footer;
    std::size_t m_main_event_type_id;
    std::unordered_map<std::uint32_t, std::size_t> m_event_types_2_stat_idx; // maps event type id to index of stats in m_event_stats
    std::vector<std::shared_ptr<ReportEventTypeStats>> m_event_stats;
};

} // namespace ReportGen
} // namespace hebench

#endif // defined _HEBench_ReportStats_H_0596d40a3cce4b108a81595c50eb286d
