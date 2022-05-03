
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "hebench_report.h"
#include "hebench_report_cpp.h"

#define INTERNAL_LOG_MSG(message) prepareLogMessage((message),                      \
                                                    __func__, m_private_class_name, \
                                                    __FILE__, __LINE__)

std::string prepareLogMessage(const char *message,
                              const char *function, const char *container,
                              const char *filename, int line_no)
{
    // prepare log message
    std::stringstream ss_retval;

    bool bheader =
        (filename && filename[0] != '\0') || (container && container[0] != '\0') || (function && function[0] != '\0') || (line_no >= 0);
    if (filename && filename[0] != '\0')
        ss_retval << filename;
    if (line_no >= 0)
        ss_retval << ":" << line_no;
    if (container && container[0] != '\0')
        ss_retval << ":" << container;
    if (function && function[0] != '\0')
        ss_retval << "::" << function << "()";
    if (bheader && message && message[0])
        ss_retval << ": ";
    if (message && message[0])
        ss_retval << message;

    return ss_retval.str();
}

std::string prepareLogMessage(const std::string &message,
                              const std::string &function, const std::string &container,
                              const std::string &filename, int line_no)
{
    return prepareLogMessage(message.c_str(),
                             function.c_str(), container.c_str(),
                             filename.c_str(), line_no);
}

namespace hebench {
namespace ReportGen {
namespace cpp {

//---------------------------
// class TimingPrefixUtility
//---------------------------

void TimingPrefixUtility::setTimingPrefix(TimingPrefixedSeconds &prefix, double seconds, char ch_prefix)
{
    hebench::ReportGen::setTimingPrefix(&prefix, seconds, ch_prefix);
}

void TimingPrefixUtility::computeTimingPrefix(TimingPrefixedSeconds &prefix, double seconds)
{
    hebench::ReportGen::computeTimingPrefix(&prefix, seconds);
}

//--------------------
// class TimingReport
//--------------------

TimingReport::TimingReport(const std::string &header) :
    m_lib_handle(nullptr)
{
    m_lib_handle = allocateReport();
    if (!m_lib_handle)
        throw std::runtime_error(INTERNAL_LOG_MSG("Error allocating report."));
    if (!header.empty()
        && !setReportHeader(m_lib_handle, header.c_str()))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new header."));
}

TimingReport::TimingReport(TimingReport &&src)
{
    m_lib_handle     = src.m_lib_handle;
    src.m_lib_handle = nullptr;
    src              = TimingReport();
}

TimingReport &TimingReport::operator=(TimingReport &&src)
{
    if (&src != this)
    {
        if (m_lib_handle)
            freeReport(m_lib_handle);
        m_lib_handle     = src.m_lib_handle;
        src.m_lib_handle = nullptr;
        src              = TimingReport();
    } // end if
    return *this;
}

TimingReport::~TimingReport()
{
    if (m_lib_handle)
        freeReport(m_lib_handle);
}

void TimingReport::setHeader(const std::string &new_header)
{
    if (!setReportHeader(m_lib_handle, new_header.c_str()))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new header."));
}

void TimingReport::appendHeader(const std::string &new_header, bool new_line)
{
    if (!appendReportHeader(m_lib_handle, new_header.c_str(), new_line ? 1 : 0))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new header."));
}

void TimingReport::prependHeader(const std::string &new_header, bool new_line)
{
    if (!prependReportHeader(m_lib_handle, new_header.c_str(), new_line ? 1 : 0))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new header."));
}

std::string TimingReport::getHeader() const
{
    std::vector<char> ch_retval;
    std::uint64_t n = getReportHeader(m_lib_handle, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving report header."));
    ch_retval.resize(n);
    if (getReportHeader(m_lib_handle, ch_retval.data(), ch_retval.size()) <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving report header."));
    return ch_retval.data();
}

void TimingReport::setFooter(const std::string &new_footer)
{
    if (!setReportFooter(m_lib_handle, new_footer.c_str()))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new footer."));
}

void TimingReport::appendFooter(const std::string &new_footer, bool new_line)
{
    if (!appendReportFooter(m_lib_handle, new_footer.c_str(), new_line ? 1 : 0))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new footer."));
}

void TimingReport::prependFooter(const std::string &new_footer, bool new_line)
{
    if (!prependReportFooter(m_lib_handle, new_footer.c_str(), new_line ? 1 : 0))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new footer."));
}

std::string TimingReport::getFooter() const
{
    std::vector<char> ch_retval;
    std::uint64_t n = getReportFooter(m_lib_handle, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving report footer."));
    ch_retval.resize(n);
    if (getReportFooter(m_lib_handle, ch_retval.data(), ch_retval.size()) <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving report footer."));
    return ch_retval.data();
}

void TimingReport::addEventType(uint32_t event_type_id, const std::string &event_type_header, bool is_main_event)
{
    bool retval = (is_main_event ?
                       hebench::ReportGen::addMainEventType(m_lib_handle, event_type_id, event_type_header.c_str()) != 0 :
                       hebench::ReportGen::addEventType(m_lib_handle, event_type_id, event_type_header.c_str()) != 0);
    if (!retval)
    {
        std::string err_msg = (is_main_event ?
                                   "Error adding main event type." :
                                   "Error adding event.");
        throw std::runtime_error(INTERNAL_LOG_MSG(err_msg));
    } // end if
}

bool TimingReport::hasEventType(uint32_t event_type_id) const
{
    int32_t retval = hebench::ReportGen::hasEventType(m_lib_handle, event_type_id);

    if (retval < 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Error querying for event type."));

    return retval > 0;
}

std::string TimingReport::getEventTypeHeader(uint32_t event_type_id) const
{
    std::vector<char> ch_retval;
    std::uint64_t n = hebench::ReportGen::getEventTypeHeader(m_lib_handle, event_type_id, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving event type header."));
    ch_retval.resize(n);
    if (hebench::ReportGen::getEventTypeHeader(m_lib_handle, event_type_id, ch_retval.data(), ch_retval.size()) <= 0)
        throw std::runtime_error(INTERNAL_LOG_MSG("Unexpected error retrieving event type header."));
    return ch_retval.data();
}

uint64_t TimingReport::getEventTypeCount() const
{
    return hebench::ReportGen::getEventTypeCount(m_lib_handle);
}

uint32_t TimingReport::getEventType(uint64_t index) const
{
    uint32_t retval = hebench::ReportGen::getEventType(m_lib_handle, index);
    if (retval == std::numeric_limits<decltype(retval)>::max())
        throw std::runtime_error(INTERNAL_LOG_MSG("Error retrieving event type ID for event type index " + std::to_string(index) + "."));
    return retval;
}

uint32_t TimingReport::getMainEventType() const
{
    uint32_t retval;

    if (!hebench::ReportGen::getMainEventType(m_lib_handle, &retval))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error retrieving main event type."));

    return retval;
}

void TimingReport::addEvent(const hebench::ReportGen::TimingReportEventC &p_event)
{
    if (!hebench::ReportGen::addEvent(m_lib_handle, &p_event))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error adding new event to report."));
}

void TimingReport::getEvent(hebench::ReportGen::TimingReportEventC &p_event, uint64_t index) const
{
    if (!hebench::ReportGen::getEvent(m_lib_handle, &p_event, index))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error retrieving event from report."));
}

uint64_t TimingReport::getEventCount() const
{
    return hebench::ReportGen::getEventCount(m_lib_handle);
}

uint64_t TimingReport::getEventCapacity() const
{
    return hebench::ReportGen::getEventCapacity(m_lib_handle);
}

void TimingReport::setEventCapacity(uint64_t new_capacity)
{
    if (!hebench::ReportGen::setEventCapacity(m_lib_handle, new_capacity))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error setting new capacity for events."));
}

void TimingReport::clear()
{
    if (!hebench::ReportGen::clearEvents(m_lib_handle))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error clearning up events."));
}

void TimingReport::save2CSV(const std::string &filename)
{
    if (!hebench::ReportGen::save2CSV(m_lib_handle, filename.c_str()))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error saving report to CSV file."));
}

std::string TimingReport::convert2CSV()
{
    char *p_tmp = nullptr;
    if (!hebench::ReportGen::convert2CSV(m_lib_handle, &p_tmp))
        throw std::runtime_error(INTERNAL_LOG_MSG("Error converting report to CSV format."));
    std::shared_ptr<char> sp_csv_content = std::shared_ptr<char>(p_tmp,
                                                                 [](char *p) {
                                                                     if (p)
                                                                         hebench::ReportGen::freeCSVContent(p);
                                                                 });
    std::string retval                   = sp_csv_content.get();
    return retval;
}

TimingReport TimingReport::loadReportFromCSV(const std::string &s_csv_content)
{
    TimingReport retval;

    char error_description[MAX_DESCRIPTION_BUFFER_SIZE];
    retval.m_lib_handle = hebench::ReportGen::loadReportFromCSV(s_csv_content.c_str(), error_description);
    if (!retval.m_lib_handle)
        throw std::runtime_error(INTERNAL_LOG_MSG(error_description));

    return retval;
}

TimingReport TimingReport::loadReportFromCSVFile(const std::string &filename)
{
    TimingReport retval;

    char error_description[MAX_DESCRIPTION_BUFFER_SIZE];
    retval.m_lib_handle = hebench::ReportGen::loadReportFromCSVFile(filename.c_str(), error_description);
    if (!retval.m_lib_handle)
        throw std::runtime_error(INTERNAL_LOG_MSG(error_description));

    return retval;
}

} // namespace cpp
} // namespace ReportGen
} // namespace hebench
