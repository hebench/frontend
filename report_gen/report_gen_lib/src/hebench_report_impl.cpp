
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <cstring>
#include <limits>
#include <sstream>

#include "hebench_report_impl.h"
#include "hebench_report_utils.h"

namespace hebench {
namespace TestHarness {
namespace Report {

std::string to_string(const std::string_view &s)
{
    return std::string(s.begin(), s.end());
}

TimingReport::TimingReport() :
    m_main_event(std::numeric_limits<decltype(m_main_event)>::max())
{
}

void TimingReport::newEventType(std::uint32_t set_id, const std::string &set_header, bool is_main_event)
{
    m_event_headers[set_id] = set_header;
    if (is_main_event || m_main_event == std::numeric_limits<decltype(m_main_event)>::max())
        m_main_event = set_id;
}

void TimingReport::newEvent(std::shared_ptr<TimingReportEventC> p_event, const std::string &set_header)
{
    if (p_event)
    {
        if (m_event_headers.count(p_event->event_type_id) <= 0 || !set_header.empty())
            newEventType(p_event->event_type_id, set_header);
        m_events.push_back(p_event);
    } // end if
}

void TimingReport::reserveCapacityForEvents(std::size_t new_capacity)
{
    m_events.reserve(new_capacity);
}

void TimingReport::clear()
{
    m_events.clear();
}

void TimingReport::appendHeader(const std::string &header, bool new_line)
{
    std::stringstream ss;
    ss << m_header;
    if (new_line)
        ss << std::endl;
    ss << header;
    m_header = ss.str();
}

void TimingReport::prependHeader(const std::string &header, bool new_line)
{
    std::stringstream ss;
    ss << header;
    if (new_line)
        ss << std::endl;
    ss << m_header;
    m_header = ss.str();
}

void TimingReport::appendFooter(const std::string &footer, bool new_line)
{
    std::stringstream ss;
    ss << m_footer;
    if (new_line)
        ss << std::endl;
    ss << footer;
    m_footer = ss.str();
}

void TimingReport::prependFooter(const std::string &footer, bool new_line)
{
    std::stringstream ss;
    ss << footer;
    if (new_line)
        ss << std::endl;
    ss << m_footer;
    m_footer = ss.str();
}

std::ostream &TimingReport::convert2CSV(std::ostream &os) const
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    os << TagVersion << std::endl // report version
       << "Events recorded," << m_events.size() << std::endl
       << "Main event," << m_main_event << std::endl
       << TagReportHeader << std::endl // header start
       << getHeader() << std::endl;
    if (!os)
        throw std::ios_base::failure("Error writing report header to stream.");
    if (m_events.empty())
    {
        os << TagFailedTest << std::endl // this report is of a test that failed validation
           << "Failed" << std::endl;
    } // end if
    else
    {
        os << TagReportData << std::endl // start of the report table
           << ",idx,ID,Event,Description,Time ratio num,Time ratio den,"
           << "Wall time start,Wall time end,Elapsed wall time,"
           << "CPU time start,CPU time end,Elapsed CPU time,Iterations"
           << std::endl;
        if (!os)
            throw std::ios_base::failure("Error writing table header to stream.");

        for (std::size_t i = 0; i < m_events.size(); ++i)
        {
            const TimingReportEventC &timing_event = *m_events[i];
            os << "," << i << "," << timing_event.event_type_id << ",";
            if (m_event_headers.count(timing_event.event_type_id) > 0)
                os << m_event_headers.at(timing_event.event_type_id);
            os << "," << timing_event.description << ","
               << timing_event.time_interval_ratio_num << "," << timing_event.time_interval_ratio_den << ","
               << timing_event.wall_time_start << "," << timing_event.wall_time_end << ","
               << timing_event.wall_time_end - timing_event.wall_time_start << ","
               << timing_event.cpu_time_start << "," << timing_event.cpu_time_end << ","
               << timing_event.cpu_time_end - timing_event.cpu_time_start << ","
               << timing_event.iterations;

            os << std::endl;

            if (!os)
                throw std::ios_base::failure("Error writing report event to stream.");
        } // end for
    } // end else

    os << TagReportFooter << std::endl // footer start
       << getFooter() << std::endl;

    // end of report
    os << TagReportEnd << std::endl;
    if (!os)
        throw std::ios_base::failure("Error writing report event to stream.");

    return os;
}

std::string_view TimingReport::findNextValueCSV(std::size_t &out_start_idx,
                                                std::size_t &out_count,
                                                const char *s)
{
    // ignore blank spaces
    std::string_view s_view(s);
    out_start_idx = std::min(s_view.find_first_not_of(" \t\n\r\f\v"), s_view.length());
    out_count     = 0;
    s_view.remove_prefix(out_start_idx);
    if (!s_view.empty() && s_view.front() != ',')
    {
        // value is not empty

        // check for quotations
        if (s_view.front() == '\"')
        {
            // find the closing quotations
            std::size_t value_end_pos = s_view.find_first_of('\"', 1);
            if (value_end_pos != std::string_view::npos)
            {
                value_end_pos = std::min(s_view.find_first_of(',', value_end_pos + 1), s_view.length());
                out_count     = value_end_pos;
            } // end if
        } // end if
        else
            out_count = std::min(s_view.find_first_of(','), s_view.length());
    } // end else

    s_view                  = std::string_view(s + out_start_idx, out_count);
    auto last_non_blank_idx = s_view.find_last_not_of(" \t\n\r\f\v");
    if (last_non_blank_idx != std::string_view::npos)
        s_view.remove_suffix(s_view.length() - last_non_blank_idx - 1);

    return s_view;
}

void TimingReport::parseHeadingValue(std::string &s_out_heading, std::uint64_t &out_value,
                                     const std::string &s_line)
{
    std::stringstream ss;
    std::size_t value_start, value_length;
    std::string_view s_line_view;
    std::string s_value;

    s_line_view = std::string_view(s_line.c_str());

    // heading
    s_out_heading = findNextValueCSV(value_start, value_length, s_line_view.data());
    if (value_start + value_length < s_line_view.length())
        ++value_length;
    s_line_view.remove_prefix(value_start + value_length);

    // value
    s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
    ss      = std::stringstream(s_value);
    if (s_value.empty() || !(ss >> out_value))
        throw std::runtime_error("Invalid CSV format. Expected type uint64_t for heading \"" + s_out_heading + "\", but read \"" + s_value + "\".");
}

void TimingReport::parseHeadingValue(std::string &s_out_heading, double &out_value,
                                     const std::string &s_line)
{
    std::stringstream ss;
    std::size_t value_start, value_length;
    std::string_view s_line_view;
    std::string s_value;

    s_line_view = std::string_view(s_line.c_str());

    // heading
    s_out_heading = findNextValueCSV(value_start, value_length, s_line_view.data());
    if (value_start + value_length < s_line_view.length())
        ++value_length;
    s_line_view.remove_prefix(value_start + value_length);

    // value
    s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
    ss      = std::stringstream(s_value);
    if (s_value.empty() || !(ss >> out_value))
        throw std::runtime_error("Invalid CSV format. Expected type double for heading \"" + s_out_heading + "\", but read \"" + s_value + "\".");
}

void TimingReport::parseTimingEvent(std::string &s_out_event_header,
                                    std::shared_ptr<TimingReportEventC> &p_out_event,
                                    const std::string &s_line)
{
    std::stringstream ss;
    std::size_t value_start, value_length;
    std::string_view s_line_view;
    std::string s_value;
    std::shared_ptr<TimingReportEventC> retval;

    s_out_event_header.clear();
    if (!s_line.empty())
    {
        std::uint32_t id;
        std::string event_description;
        double cpu_start_time, cpu_end_time, wall_start_time, wall_end_time;
        int64_t time_interval_num, time_interval_den;
        std::uint64_t iterations;

        s_line_view = std::string_view(s_line.c_str());

        // parse order:
        // ,idx,ID,Event,Description,Time ratio num, Time ratio den,Wall time start,Wall time end,Elapsed wall time,CPU time start,CPU time end,Elapsed CPU time,Iterations"

        // skip any empty columns at the start
        std::string_view sv_tmp;
        do
        {
            sv_tmp = findNextValueCSV(value_start, value_length, s_line_view.data());
            if (sv_tmp.empty())
            {
                if (value_start + value_length < s_line_view.length())
                    ++value_length;
                s_line_view.remove_prefix(value_start + value_length);
            } // end if
        } while (sv_tmp.empty());

        // idx (skip)
        findNextValueCSV(value_start, value_length, s_line_view.data());
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);

        // ID
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> id))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type uint32_t for ID, but read value \"" + s_value + "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // Event
        s_out_event_header = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);

        // Description
        event_description = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);

        // Time ratio num
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> time_interval_num))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type int64 for Time ratio num, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // Time ratio den
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> time_interval_den) || time_interval_den == 0)
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected non-zero of type int64 for Time ratio den, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // Wall time start
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> wall_start_time))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type double for Wall time start, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // Wall time end
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> wall_end_time))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type double for Wall time end, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // Wall time elapsed (skip)
        findNextValueCSV(value_start, value_length, s_line_view.data());
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);

        // CPU time start
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> cpu_start_time))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type double for CPU time start, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // CPU time end
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> cpu_end_time))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type double for CPU time end, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        // CPU time elapsed (skip)
        findNextValueCSV(value_start, value_length, s_line_view.data());
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);

        // Iterations
        s_value = to_string(findNextValueCSV(value_start, value_length, s_line_view.data()));
        if (value_start + value_length < s_line_view.length())
            ++value_length;
        s_line_view.remove_prefix(value_start + value_length);
        ss = std::stringstream(s_value);
        if (s_value.empty() || !(ss >> iterations))
        {
            ss = std::stringstream();
            ss << "Invalid timing event format. Expected type uint64_t for Iterations, but read value \"" << s_value << "\".";
            throw std::runtime_error(ss.str());
        } // end if

        retval                = std::make_shared<TimingReportEventC>();
        retval->event_type_id = id;
        hebench::Utilities::copyString(retval->description, MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE, event_description);
        retval->cpu_time_start          = cpu_start_time;
        retval->cpu_time_end            = cpu_end_time;
        retval->wall_time_start         = wall_start_time;
        retval->wall_time_end           = wall_end_time;
        retval->iterations              = iterations;
        retval->time_interval_ratio_num = time_interval_num;
        retval->time_interval_ratio_den = time_interval_den;

    } // end if

    p_out_event = retval;
}

std::istream &TimingReport::getTrimmedLine(std::istream &is, std::string &s_out, const std::string &extra_trim)
{
    return getTrimmedLine(is, s_out, extra_trim, extra_trim);
}

std::istream &TimingReport::getTrimmedLine(std::istream &is, std::string &s_out,
                                           const std::string &extra_ltrim, const std::string &extra_rtrim)
{
    std::getline(is, s_out);
    std::string s_ltrim(" \t\n\r\f\v");
    std::string s_rtrim = s_ltrim;
    if (!extra_ltrim.empty())
        s_ltrim.append(extra_ltrim);
    if (!extra_rtrim.empty())
        s_rtrim.append(extra_rtrim);
    s_out.erase(0, s_out.find_first_not_of(s_ltrim));
    s_out.erase(s_out.find_last_not_of(s_rtrim) + 1);
    return is;
}

std::string TimingReport::readTextBlock(std::istream &is, std::string &s_block,
                                        const std::vector<std::string> tags)
{
    constexpr const char *ch_trim = " \t\n\r\f\v,";
    std::string s_line;
    std::string s_tag;
    std::stringstream ss;
    bool b_found            = false;
    bool b_first_block_line = true;

    while (is && !b_found)
    {
        std::getline(is, s_line);
        s_tag = s_line;
        s_tag.erase(0, s_tag.find_first_not_of(ch_trim));
        s_tag.erase(s_tag.find_last_not_of(ch_trim) + 1);

        for (std::size_t tag_i = 0; !b_found && tag_i < tags.size(); ++tag_i)
            b_found = s_tag == tags[tag_i];
        if (!b_found)
        {
            if (b_first_block_line)
                b_first_block_line = false;
            else
                ss << std::endl;
            ss << s_line;
        } // end if
    } // end while

    s_block = ss.str();
    return b_found ? s_tag : std::string();
}

TimingReport TimingReport::loadCSV(const std::string &csv_content)
{
    std::istringstream iss(csv_content);
    return loadCSV(iss);
}

TimingReport TimingReport::loadCSV(std::istream &is)
{
    std::string s_line;
    std::string s_block;
    std::uint64_t events_count, u64_main_event;

    TimingReport retval;

    // version
    getTrimmedLine(is, s_line, ",");
    if (s_line != TagVersion)
    {
        std::stringstream ss;
        ss << "Invalid CSV report version found. Expected \"" << TagVersion << "\", but read \"" << s_line << "\".";
        throw std::runtime_error(ss.str());
    } // end if

    // read events recorded
    getTrimmedLine(is, s_line, ",");
    std::string heading;
    parseHeadingValue(heading, events_count, s_line);

    // read main event
    getTrimmedLine(is, s_line, ",");
    parseHeadingValue(heading, u64_main_event, s_line);

    // read until test header found
    if (TagReportHeader != readTextBlock(is, s_block, { TagReportHeader }))
        throw std::runtime_error("Report header not found in CSV. End of file reached.");

    // read header
    s_line = readTextBlock(is, s_block, { TagReportData, TagFailedTest });
    retval.setHeader(s_block);

    if (s_line == TagFailedTest) // report is a validation failed run
    {
        // skip content until footer is found
        while (s_line != TagReportFooter && (is))
            getTrimmedLine(is, s_line, ",");
    } // end if
    else if (s_line == TagReportData) // report is a valid run
    {
        // skip table header
        getTrimmedLine(is, s_line, ",");

        // add main event
        retval.newEventType(u64_main_event, "", true);

        // read each timing event until footer is found
        while (s_line != TagReportFooter && (is))
        {
            getTrimmedLine(is, s_line, ",");

            if (s_line != TagReportFooter)
            {
                std::string s_event_header;
                std::shared_ptr<TimingReportEventC> p_event;
                parseTimingEvent(s_event_header, p_event, s_line);
                retval.newEvent(p_event, s_event_header);
            } // end if
        } // end while

        if (retval.getEvents().size() != events_count)
        {
            std::stringstream ss;
            ss << "Inconsistent number of events read from CSV. Expected " << events_count << ", but read " << retval.getEvents().size() << ".";
            throw std::runtime_error(ss.str());
        } // end if
    } // end else
    else
        throw std::runtime_error("Report data not found in CSV. End of file reached.");

    if (s_line != TagReportFooter)
        throw std::runtime_error("Report footer not found in CSV. End of file reached.");

    // read footer
    s_line = readTextBlock(is, s_block, { TagReportEnd });
    //    if (TagReportEnd != s_line)
    //        throw std::runtime_error("Report end marker not found in CSV. End of file reached.");
    retval.setFooter(s_block);

    return retval;
}

void TimingReport::computeTimingPrefix(TimingPrefixedSeconds &prefix, double seconds)
{
    // convert to seconds
    prefix.value                   = seconds;
    prefix.time_interval_ratio_den = 1;
    std::size_t scale_i;
    for (scale_i = 0; scale_i < 4 && prefix.value > 0.0 && prefix.value < 1.0; ++scale_i)
    {
        prefix.value *= 1000.0;
        prefix.time_interval_ratio_den *= 1000;
    } // end for

    switch (scale_i)
    {
    case 0: // seconds
        hebench::Utilities::copyString(prefix.symbol, MAX_SYMBOL_BUFFER_SIZE, "");
        hebench::Utilities::copyString(prefix.prefix, MAX_DESCRIPTION_BUFFER_SIZE, "");
        break;

    case 1: // milliseconds
        hebench::Utilities::copyString(prefix.symbol, MAX_SYMBOL_BUFFER_SIZE, "m");
        hebench::Utilities::copyString(prefix.prefix, MAX_DESCRIPTION_BUFFER_SIZE, "milli");
        break;

    case 2: // microseconds
        hebench::Utilities::copyString(prefix.symbol, MAX_SYMBOL_BUFFER_SIZE, "u");
        hebench::Utilities::copyString(prefix.prefix, MAX_DESCRIPTION_BUFFER_SIZE, "micro");
        break;

    default: // 3 - nanoseconds
        hebench::Utilities::copyString(prefix.symbol, MAX_SYMBOL_BUFFER_SIZE, "n");
        hebench::Utilities::copyString(prefix.prefix, MAX_DESCRIPTION_BUFFER_SIZE, "nano");
        break;
    } // end switch
}

//---------------------
// class ReportSummary
//---------------------

void ReportSummary::generateCSV(std::ostream &os, TimingReportEventC &main_event_summary, const TimingReport &report)
{
    if (!os)
        throw std::ios_base::failure("Output stream is in an invalid state.");

    std::memset(&main_event_summary, 0, sizeof(TimingReportEventC));

    struct LocalStats
    {
        hebench::Utilities::Math::EventStats ave_wall;
        hebench::Utilities::Math::EventStats ave_cpu;
    };
    std::unordered_map<decltype(TimingReportEventC::event_type_id), LocalStats> stats; // std::uint32_t
    std::vector<decltype(TimingReportEventC::event_type_id)> event_order;

    // compute the stats on the events
    for (std::shared_ptr<TimingReportEventC> p_event : report.getEvents())
    {
        if (stats.count(p_event->event_type_id) <= 0)
        {
            stats[p_event->event_type_id] = LocalStats();
            event_order.push_back(p_event->event_type_id);
        } // end if

        double wall_time = TimingReport::computeElapsedWallTime(*p_event) / p_event->iterations;
        double cpu_time  = TimingReport::computeElapsedCPUTime(*p_event) / p_event->iterations;
        for (std::uint64_t i = 0; i < p_event->iterations; ++i)
        {
            stats[p_event->event_type_id].ave_wall.newEvent(wall_time);
            stats[p_event->event_type_id].ave_cpu.newEvent(cpu_time);
        } // end for
    } // end for

    os << report.getHeader() << std::endl
       << std::endl
       << "Notes" << std::endl
       << report.getFooter() << std::endl
       << std::endl
       << "Main event," << report.getMainEventID() << "," << report.getEventTypes().at(report.getMainEventID()) << std::endl
       << std::endl
       << "ID,Event,Average Wall time,Unit,Variance Wall time,Average CPU time,Unit,Variance CPU time,Iterations" << std::endl;
    if (!os)
        throw std::ios_base::failure("Error writing summary report header to stream.");
    for (std::size_t i = 0; i < event_order.size(); ++i)
    {
        auto id                       = event_order[i];
        const LocalStats &event_stats = stats.at(id);
        assert(event_stats.ave_wall.getCount() == event_stats.ave_cpu.getCount());
        hebench::TestHarness::Report::TimingPrefixedSeconds prefix_wall;
        hebench::TestHarness::Report::TimingPrefixedSeconds prefix_cpu;
        hebench::TestHarness::Report::TimingReport::computeTimingPrefix(prefix_wall, event_stats.ave_wall.getMean());
        hebench::TestHarness::Report::TimingReport::computeTimingPrefix(prefix_cpu, event_stats.ave_cpu.getMean());
        os << id << "," << report.getEventTypes().at(id) << ","
           << event_stats.ave_wall.getMean() * prefix_wall.time_interval_ratio_den << "," << prefix_wall.symbol << "s,"
           << event_stats.ave_wall.getVariance() * prefix_wall.time_interval_ratio_den * prefix_wall.time_interval_ratio_den << ","
           << event_stats.ave_cpu.getMean() * prefix_cpu.time_interval_ratio_den << "," << prefix_cpu.symbol << "s,"
           << event_stats.ave_cpu.getVariance() * prefix_cpu.time_interval_ratio_den * prefix_cpu.time_interval_ratio_den << ","
           << event_stats.ave_wall.getCount() << std::endl;
        if (!os)
            throw std::ios_base::failure("Error writing summary row to stream.");
        if (i == 0 || report.getMainEventID() == id)
        {
            main_event_summary.event_type_id           = id;
            main_event_summary.cpu_time_start          = 0;
            main_event_summary.cpu_time_end            = event_stats.ave_cpu.getMean();
            main_event_summary.wall_time_start         = 0;
            main_event_summary.wall_time_end           = event_stats.ave_wall.getMean();
            main_event_summary.iterations              = event_stats.ave_wall.getCount();
            main_event_summary.time_interval_ratio_num = 1; // 1:1 seconds
            main_event_summary.time_interval_ratio_den = 1;
            hebench::Utilities::copyString(main_event_summary.description,
                                           MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE,
                                           report.getEventTypes().at(id));
        } // end if
    } // end for
}

} // namespace Report
} // namespace TestHarness
} // namespace hebench
