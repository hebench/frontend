
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <unordered_set>

#include "hebench_report_stats.h"
#include "modules/general/include/hebench_math_utils.h"
#include "modules/general/include/hebench_utilities.h"

namespace hebench {
namespace ReportGen {

void computeStats(StatisticsResult &result, const double *data, std::size_t count)
{
    std::vector<double> sorted_data(data, data + count);
    std::sort(sorted_data.begin(), sorted_data.end());
    std::size_t trim_start = sorted_data.size() / 10;
    //std::size_t trim_count = sorted_data.size() - 2 * trim_start;

    hebench::Utilities::Math::EventStats basic_stats;
    for (std::size_t i = 0; i < count; ++i)
        basic_stats.newEvent(data[i]);
    hebench::Utilities::Math::EventStats trimmed_stats;
    for (std::size_t i = trim_start; i < sorted_data.size() - trim_start; ++i)
        trimmed_stats.newEvent(sorted_data[i]);

    std::memset(&result, 0, sizeof(StatisticsResult));
    result.total    = basic_stats.getTotal();
    result.ave      = basic_stats.getMean();
    result.variance = basic_stats.getVariance();
    result.min      = basic_stats.getMin();
    result.max      = basic_stats.getMax();

    result.median = hebench::Utilities::Math::computePercentile(sorted_data.data(), sorted_data.size(), 0.5);
    result.pct_1  = hebench::Utilities::Math::computePercentile(sorted_data.data(), sorted_data.size(), 0.01); // 1-th percentile
    result.pct_10 = hebench::Utilities::Math::computePercentile(sorted_data.data(), sorted_data.size(), 0.1); // 10-th percentile
    result.pct_90 = hebench::Utilities::Math::computePercentile(sorted_data.data(), sorted_data.size(), 0.9); // 90-th percentile
    result.pct_99 = hebench::Utilities::Math::computePercentile(sorted_data.data(), sorted_data.size(), 0.95); // 99-th percentile

    result.ave_trim              = trimmed_stats.getMean(); // trimmed by 10% on each side
    result.variance_trim         = trimmed_stats.getVariance();
    result.samples_per_unit      = (basic_stats.getTotal() == 0.0 ? 0.0 : basic_stats.getCount() / basic_stats.getTotal()); // = total / iterations
    result.samples_per_unit_trim = (trimmed_stats.getTotal() == 0.0 ? 0.0 : trimmed_stats.getCount() / trimmed_stats.getTotal()); // = total_trim / iterations_trim
    result.input_sample_count    = basic_stats.getCount();
}

/**
 * @brief Extracts and maintains the timing report events of the same type.
 * @details Maintains a collection of timing report events of the same type
 * extracted from a report. The events' timings are converted to elapsed
 * times in seconds.
 */
class EventType
{
public:
    /**
     * @brief Constructs an event type from a report.
     * @param[in] report Report from which to extract the events.
     * @param[in] event_id ID of type of event to extract.
     */
    EventType(const cpp::TimingReport &report, std::uint32_t event_id);
    EventType(const std::vector<double> &cpu_events, const std::vector<double> &wall_events,
              std::uint32_t event_id, const std::string_view &event_name);

    /**
     * @brief ID of the event type.
     */
    std::uint32_t getID() const { return m_id; }
    /**
     * @brief Name of the event type as per the report.
     */
    const std::string &getName() const { return m_name; }
    /**
     * @brief Collection of contained CPU-timed events of the same type extracted from the report.
     */
    const std::vector<double> &getCPUEvents() const { return m_cpu_events; }
    /**
     * @brief Collection of contained wall-timed events of the same type extracted from the report.
     */
    const std::vector<double> &getWallEvents() const { return m_wall_events; }

    /**
     * @brief Computes statistics for this event type based on the contained events.
     * @param[out] result ReportEventTypeStats to receive the statistical computation
     * results.
     * @details All timings for the stats are in seconds.
     */
    void computeStats(ReportEventTypeStats &result) const;
    /**
     * @brief Computes statistics for this event type based on the contained events.
     * @return An instance of ReportEventTypeStats containing the statistical
     * computation results.
     * @details All timings for the stats are in seconds.
     */
    ReportEventTypeStats computeStats() const
    {
        ReportEventTypeStats retval;
        computeStats(retval);
        return retval;
    }

private:
    std::uint32_t m_id;
    std::string m_name;
    std::vector<double> m_cpu_events;
    std::vector<double> m_wall_events;
};

EventType::EventType(const cpp::TimingReport &report, std::uint32_t event_id)
{
    m_id   = event_id;
    m_name = report.getEventTypeHeader(event_id);

    std::exception_ptr p_ex;

    //#pragma omp parallel for
    for (std::uint64_t event_i = 0; event_i < report.getEventCount(); ++event_i)
    {
        if (!p_ex)
        {
            try
            {
                TimingReportEventC event;
                report.getEvent(event, event_i);
                if (!p_ex && event.event_type_id == event_id)
                {
                    double wall_time = cpp::TimingReport::computeElapsedWallTime(event) / event.input_sample_count;
                    double cpu_time  = cpp::TimingReport::computeElapsedCPUTime(event) / event.input_sample_count;
                    //#pragma omp critical
                    {
                        try
                        {
                            m_wall_events.insert(m_wall_events.end(), event.input_sample_count, wall_time);
                            m_cpu_events.insert(m_cpu_events.end(), event.input_sample_count, cpu_time);
                        }
                        catch (...)
                        {
                            p_ex = std::current_exception();
                        }
                    }
                } // end if
            }
            catch (...)
            {
                p_ex = std::current_exception();
            }
        } // end if
    } // end for

    if (p_ex)
        std::rethrow_exception(p_ex);
}

EventType::EventType(const std::vector<double> &cpu_events, const std::vector<double> &wall_events,
                     std::uint32_t event_id, const std::string_view &event_name)
{
    if (cpu_events.size() != wall_events.size())
        throw std::invalid_argument("Number of CPU events and Wall events cannot differ.");

    m_id          = event_id;
    m_name        = std::string(event_name.begin(), event_name.end());
    m_cpu_events  = cpu_events;
    m_wall_events = wall_events;
}

void EventType::computeStats(ReportEventTypeStats &result) const
{
    StatisticsResult stats;

    hebench::ReportGen::computeStats(stats, this->getCPUEvents().data(), this->getCPUEvents().size());
    result.cpu_time_ave           = stats.ave;
    result.cpu_time_variance      = stats.variance;
    result.cpu_time_min           = stats.min;
    result.cpu_time_max           = stats.max;
    result.cpu_time_median        = stats.median;
    result.cpu_time_1             = stats.pct_1;
    result.cpu_time_10            = stats.pct_10;
    result.cpu_time_90            = stats.pct_90;
    result.cpu_time_99            = stats.pct_99;
    result.cpu_time_ave_trim      = stats.ave_trim;
    result.cpu_time_variance_trim = stats.variance_trim;

    hebench::ReportGen::computeStats(stats, this->getWallEvents().data(), this->getWallEvents().size());
    result.wall_time_ave           = stats.ave;
    result.wall_time_variance      = stats.variance;
    result.wall_time_min           = stats.min;
    result.wall_time_max           = stats.max;
    result.wall_time_median        = stats.median;
    result.wall_time_1             = stats.pct_1;
    result.wall_time_10            = stats.pct_10;
    result.wall_time_90            = stats.pct_90;
    result.wall_time_99            = stats.pct_99;
    result.wall_time_ave_trim      = stats.ave_trim;
    result.wall_time_variance_trim = stats.variance_trim;
    result.ops_per_sec             = stats.samples_per_unit;
    result.ops_per_sec_trim        = stats.samples_per_unit_trim;
    result.total_time              = stats.total;
    result.input_sample_count      = stats.input_sample_count;

    result.event_id    = this->getID();
    result.name        = this->getName();
    result.description = std::string();
}

ReportStats::ReportStats(const cpp::TimingReport &report)
{
    if (report.getEventCount() <= 0)
        throw std::invalid_argument("Report belongs to a failed benchmark.");

    m_header             = report.getHeader();
    m_footer             = report.getFooter();
    m_main_event_type_id = report.getMainEventType();
    m_event_stats.reserve(report.getEventTypeCount());

    std::vector<std::uint32_t> event_ids; // used to keep track of all event ids
    event_ids.reserve(report.getEventTypeCount());

    // map event ID to the event timings
    std::unordered_map<std::uint32_t, std::vector<double>> cpu_events;
    std::unordered_map<std::uint32_t, std::vector<double>> wall_events;

    // retrieve the timings and group by event (use a single pass over the report)
    bool b_added;
    for (std::uint64_t event_i = 0; event_i < report.getEventCount(); ++event_i)
    {
        hebench::ReportGen::TimingReportEventC event;
        report.getEvent(event, event_i);
        double cpu_time  = cpp::TimingReport::computeElapsedCPUTime(event) / event.input_sample_count;
        double wall_time = cpp::TimingReport::computeElapsedWallTime(event) / event.input_sample_count;
        b_added          = false;
        if (cpu_events.count(event.event_type_id) <= 0)
        {
            cpu_events[event.event_type_id] = std::vector<double>();
            b_added                         = true;
        } // end if
        if (wall_events.count(event.event_type_id) <= 0)
        {
            wall_events[event.event_type_id] = std::vector<double>();
            b_added                          = true;
        } // end if
        if (b_added)
            event_ids.push_back(event.event_type_id);
        for (std::uint64_t i = 0; i < event.input_sample_count; ++i)
        {
            cpu_events[event.event_type_id].push_back(cpu_time);
            wall_events[event.event_type_id].push_back(wall_time);
        } // end for
    } // end for

    // sort by event ID
    std::sort(event_ids.begin(), event_ids.end());

    // compute the stats for each event timing
    for (std::size_t event_id_i = 0; event_id_i < event_ids.size(); ++event_id_i)
    {
        std::uint32_t event_id = event_ids[event_id_i];
        EventType event_type(cpu_events[event_id], wall_events[event_id], event_id, report.getEventTypeHeader(event_id));
        m_event_types_2_stat_idx[event_type.getID()]  = event_id_i; // record the event type ID and match it to the index of the stats
        std::shared_ptr<ReportEventTypeStats> p_stats = std::make_shared<ReportEventTypeStats>();
        event_type.computeStats(*p_stats);
        m_event_stats.push_back(p_stats);
    } // end for
}

const ReportEventTypeStats &ReportStats::getEventTypeStats(std::uint64_t index) const
{
    if (index >= m_event_stats.size())
    {
        std::stringstream ss;
        ss << "Out of range `index`. Received " << index << ", but expected less than " << m_event_stats.size() << ".";
        throw std::out_of_range(ss.str());
    } // end if
    if (!m_event_stats[index])
        throw std::runtime_error("Unexpected empty stats.");
    return *m_event_stats[index];
}

const ReportEventTypeStats &ReportStats::getEventTypeStatsByID(std::uint32_t id) const
{
    return getEventTypeStats(m_event_types_2_stat_idx.at(id));
}

std::uint64_t ReportStats::getMainEventTypeStatsIndex() const
{
    return m_event_types_2_stat_idx.at(m_main_event_type_id);
}

const ReportEventTypeStats &ReportStats::getMainEventTypeStats() const
{
    return getEventTypeStatsByID(m_main_event_type_id);
}

void ReportStats::generateCSV(std::ostream &os, char ch_prefix)
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    os << this->getHeader() << std::endl
       << std::endl
       << "Notes" << std::endl
       << this->getFooter() << std::endl
       << std::endl
       << "Main event," << this->getMainEventTypeStats().event_id << "," << this->getMainEventTypeStats().name << std::endl
       << std::endl
       << ",,,,,Wall Time,,,,,,,,,,,,,CPU Time" << std::endl
       << "ID,Event,Total Wall Time,Samples per sec,Samples per sec trimmed,"
       // wall
       << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,"
       // cpu
       << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Input Samples" << std::endl;
    if (!os)
        throw std::ios_base::failure("Error writing statistics report header to stream.");
    for (std::uint64_t event_stats_i = 0; event_stats_i < m_event_stats.size(); ++event_stats_i)
        if (m_event_stats[event_stats_i])
            generateCSV(os, *m_event_stats[event_stats_i], ch_prefix);
}

void ReportStats::generateCSV(std::ostream &os, const ReportEventTypeStats &stats, char ch_prefix, bool new_line)
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    hebench::ReportGen::TimingPrefixedSeconds prefix_wall;
    hebench::ReportGen::TimingPrefixedSeconds prefix_cpu;
    hebench::ReportGen::cpp::TimingPrefixUtility::setTimingPrefix(prefix_wall, stats.wall_time_ave, ch_prefix);
    hebench::ReportGen::cpp::TimingPrefixUtility::setTimingPrefix(prefix_cpu, stats.cpu_time_ave, ch_prefix);

    os << stats.event_id << "," << stats.name << ","
       << hebench::Utilities::convertDoubleToStr(stats.total_time * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.ops_per_sec) << ","
       << hebench::Utilities::convertDoubleToStr(stats.ops_per_sec_trim) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_ave * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.wall_time_variance) * prefix_wall.time_interval_ratio_den) << ","
       << prefix_wall.symbol << "s," << hebench::Utilities::convertDoubleToStr(1.0 / prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_min * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_max * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_median * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_ave_trim * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.wall_time_variance_trim) * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_1 * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_10 * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_90 * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_99 * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_ave * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.cpu_time_variance) * prefix_cpu.time_interval_ratio_den) << ","
       << prefix_cpu.symbol << "s," << hebench::Utilities::convertDoubleToStr(1.0 / prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_min * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_max * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_median * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_ave_trim * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.cpu_time_variance_trim) * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_1 * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_10 * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_90 * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_99 * prefix_cpu.time_interval_ratio_den) << ","
       << stats.input_sample_count;
    if (new_line)
        os << std::endl;

    if (!os)
        throw std::ios_base::failure("Error writing statistics row to stream.");
}

void ReportStats::generateSummaryCSV(std::ostream &os, char ch_prefix)
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    os << this->getHeader() << std::endl
       << std::endl
       << "Notes" << std::endl
       << this->getFooter() << std::endl
       << std::endl
       << "Main event," << this->getMainEventTypeStats().event_id << "," << this->getMainEventTypeStats().name << std::endl
       << std::endl
       << ",,,Wall Time,,,,CPU Time" << std::endl
       << "ID,Event,Samples per sec,"
       << "Average,Standard Deviation,Time Unit,Time Factor,"
       << "Average,Standard Deviation,Time Unit,Time Factor,Input Samples" << std::endl;
    if (!os)
        throw std::ios_base::failure("Error writing summary report header to stream.");
    for (std::uint64_t event_stats_i = 0; event_stats_i < m_event_stats.size(); ++event_stats_i)
        if (m_event_stats[event_stats_i])
            generateSummaryCSV(os, *m_event_stats[event_stats_i], ch_prefix);
}

void ReportStats::generateSummaryCSV(std::ostream &os, const ReportEventTypeStats &stats, char ch_prefix, bool new_line)
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    hebench::ReportGen::TimingPrefixedSeconds prefix_wall;
    hebench::ReportGen::TimingPrefixedSeconds prefix_cpu;
    hebench::ReportGen::cpp::TimingPrefixUtility::setTimingPrefix(prefix_wall, stats.wall_time_ave, ch_prefix);
    hebench::ReportGen::cpp::TimingPrefixUtility::setTimingPrefix(prefix_cpu, stats.cpu_time_ave, ch_prefix);
    os << stats.event_id << "," << stats.name << ","
       << hebench::Utilities::convertDoubleToStr(stats.ops_per_sec) << ","
       << hebench::Utilities::convertDoubleToStr(stats.wall_time_ave * prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.wall_time_variance) * prefix_wall.time_interval_ratio_den) << ","
       << prefix_wall.symbol << "s," << hebench::Utilities::convertDoubleToStr(1.0 / prefix_wall.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(stats.cpu_time_ave * prefix_cpu.time_interval_ratio_den) << ","
       << hebench::Utilities::convertDoubleToStr(std::sqrt(stats.cpu_time_variance) * prefix_cpu.time_interval_ratio_den) << ","
       << prefix_cpu.symbol << "s," << hebench::Utilities::convertDoubleToStr(1.0 / prefix_cpu.time_interval_ratio_den) << ","
       << stats.input_sample_count;
    if (new_line)
        os << std::endl;

    if (!os)
        throw std::ios_base::failure("Error writing summary row to stream.");
}

} // namespace ReportGen
} // namespace hebench
