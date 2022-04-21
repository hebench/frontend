
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>

#include "hebench_report_stats.h"
#include "modules/general/include/hebench_math_utils.h"
#include "modules/general/include/hebench_utilities.h"

namespace hebench {
namespace ReportGen {

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

#pragma omp parallel for
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
                    double wall_time = cpp::TimingReport::computeElapsedWallTime(event) / event.iterations;
                    double cpu_time  = cpp::TimingReport::computeElapsedCPUTime(event) / event.iterations;
#pragma omp critical
                    {
                        try
                        {
                            m_wall_events.insert(m_wall_events.end(), event.iterations, wall_time);
                            m_cpu_events.insert(m_cpu_events.end(), event.iterations, cpu_time);
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

void EventType::computeStats(ReportEventTypeStats &result) const
{
    (void)result;
    throw std::runtime_error("Not implemented.");
}

////------------------------------
//// class ReportMainEventSummary
////------------------------------

//ReportMainEventSummary::ReportMainEventSummary(const TimingReport &report)
//{
//    hebench::Utilities::Math::EventStats stats_wall;
//    hebench::Utilities::Math::EventStats stats_cpu;

//    // compute the stats on the main event
//    for (const std::shared_ptr<TimingReportEventC> &p_event : report.getEvents())
//    {
//        if (p_event->event_type_id == report.getMainEventID())
//        {
//            double wall_time = TimingReport::computeElapsedWallTime(*p_event) / p_event->iterations;
//            double cpu_time  = TimingReport::computeElapsedCPUTime(*p_event) / p_event->iterations;
//            for (std::uint64_t i = 0; i < p_event->iterations; ++i)
//            {
//                stats_wall.newEvent(wall_time);
//                stats_cpu.newEvent(cpu_time);
//            } // end for
//        } // end if
//    } // end for

//    m_event_summary.event_id = report.getMainEventID();
//    m_event_summary.iterations = stats_wall.getCount();
//    m_event_summary.total_time = stats_wall.getTotal();
//    m_event_summary.wall_time_ave = stats_wall.getMean();
//    m_event_summary.wall_time_max = stats_wall.getMax();
//    m_event_summary.wall_time_min = stats_wall.getMin();
//    m_event_summary.wall_time_variance = stats_wall.getVariance();
//    m_event_summary.cpu_time_ave = stats_cpu.getMean();
//    m_event_summary.cpu_time_max = stats_cpu.getMax();
//    m_event_summary.cpu_time_min = stats_cpu.getMin();
//    m_event_summary.cpu_time_variance = stats_cpu.getVariance();
//    hebench::Utilities::copyString(m_event_summary.name,
//                                   MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE,
//                                   report.getEventTypes().at(m_event_summary.event_id));
//}

//-------------------
// class ReportStats
//-------------------

//ReportStats::ReportStats(const cpp::TimingReport &report) :
//    m_main_event_index(std::numeric_limits<decltype(m_main_event_index)>::max())
//{
//    // all timings for the stats are in seconds

//    m_header = report.getHeader();
//    m_footer = report.getFooter();

//    struct LocalStats
//    {
//        hebench::Utilities::Math::EventStats ave_wall;
//        hebench::Utilities::Math::EventStats ave_cpu;
//    };
//    std::unordered_map<decltype(TimingReportEventC::event_type_id), LocalStats> stats; // std::uint32_t
//    std::vector<decltype(TimingReportEventC::event_type_id)> event_order;

//    report.

//    // compute the stats on the events
//    for (const std::shared_ptr<TimingReportEventC> &p_event : report.getEvents())
//    {
//        if (stats.count(p_event->event_type_id) <= 0)
//        {
//            stats[p_event->event_type_id] = LocalStats();
//            event_order.push_back(p_event->event_type_id);
//            if (p_event->event_type_id == report.getMainEventID())
//                m_main_event_index = event_order.size() - 1;
//        } // end if

//        double wall_time = TimingReport::computeElapsedWallTime(*p_event) / p_event->iterations;
//        double cpu_time  = TimingReport::computeElapsedCPUTime(*p_event) / p_event->iterations;
//        for (std::uint64_t i = 0; i < p_event->iterations; ++i)
//        {
//            stats[p_event->event_type_id].ave_wall.newEvent(wall_time);
//            stats[p_event->event_type_id].ave_cpu.newEvent(cpu_time);
//        } // end for
//    } // end for

//    std::sort(event_order.begin(), event_order.end());

//    for (std::size_t i = 0; i < event_order.size(); ++i)
//    {
//        auto id                       = event_order[i];
//        const LocalStats &event_stats = stats.at(id);
//        assert(event_stats.ave_wall.getCount() == event_stats.ave_cpu.getCount());

//        m_event_summaries.emplace_back(std::make_shared<TimingReportEventSummaryC>());
//        TimingReportEventSummaryC &event_summary = *m_event_summaries.back();
//        std::memset(&event_summary, 0, sizeof(TimingReportEventSummaryC));
//        event_summary.event_id           = id;
//        event_summary.cpu_time_ave       = event_stats.ave_cpu.getMean();
//        event_summary.cpu_time_variance  = event_stats.ave_cpu.getVariance();
//        event_summary.wall_time_ave      = event_stats.ave_wall.getMean();
//        event_summary.wall_time_variance = event_stats.ave_wall.getVariance();
//        event_summary.iterations         = event_stats.ave_wall.getCount();
//        hebench::Utilities::copyString(event_summary.name,
//                                       MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE,
//                                       report.getEventTypes().at(id));
//    } // end for
//}

ReportStats::ReportStats(const cpp::TimingReport &report)
{
    if (report.getEventCount() <= 0)
        throw std::invalid_argument("Report belongs to a failed benchmark.");

    m_header             = report.getHeader();
    m_footer             = report.getFooter();
    m_main_event_type_id = report.getMainEventType();
    m_event_stats.reserve(report.getEventTypeCount());
    for (std::uint64_t event_type_i = 0; event_type_i < report.getEventTypeCount(); ++event_type_i)
    {
        EventType event_type(report, report.getEventType(event_type_i));
        m_event_types_2_stat_idx[event_type.getID()]  = m_event_stats.size(); // record the event type ID and match it to the index of the stats
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
       << "ID,Event,Total Wall Time,Ops per sec,Ops per sec trimmed,"
       // wall
       << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,"
       // cpu
       << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Iterations" << std::endl;
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
       << stats.iterations;
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
       << "ID,Event,Ops per sec,"
       << "Average,Standard Deviation,Time Unit,Time Factor,"
       << "Average,Standard Deviation,Time Unit,Time Factor,Iterations" << std::endl;
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
       << stats.iterations;
    if (new_line)
        os << std::endl;

    if (!os)
        throw std::ios_base::failure("Error writing summary row to stream.");
}

} // namespace ReportGen
} // namespace hebench
