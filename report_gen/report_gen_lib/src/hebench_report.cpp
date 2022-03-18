
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstring>
#include <fstream>
#include <new>
#include <sstream>
#include <string>

#include "hebench_report.h"
#include "hebench_report_impl.h"
#include "hebench_report_utils.h"

namespace hebench {
namespace TestHarness {
namespace Report {

extern "C"
{

    void *allocateReport()
    {
        TimingReport *p_retval = nullptr;

        try
        {
            p_retval = new TimingReport();
        }
        catch (...)
        {
            if (p_retval)
            {
                delete p_retval;
                p_retval = nullptr;
            } // end if
        }

        return p_retval;
    }

    void freeReport(void *p_report)
    {
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (p)
                delete p;
        }
        catch (...)
        {
            // do not throw in C
        }
    }

    int32_t setReportHeader(void *p_report, const char *new_header)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->setHeader(new_header ? new_header : "");

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t appendReportHeader(void *p_report, const char *new_header, int32_t new_line)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->appendHeader(new_header ? new_header : "", new_line != 0);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t prependReportHeader(void *p_report, const char *new_header, int32_t new_line)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->prependHeader(new_header ? new_header : "", new_line != 0);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    uint64_t getReportHeader(void *p_report, char *header, uint64_t size)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = hebench::Utilities::copyString(header, size, p->getHeader());
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t setReportFooter(void *p_report, const char *new_footer)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->setFooter(new_footer ? new_footer : "");

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t appendReportFooter(void *p_report, const char *new_footer, int32_t new_line)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->appendFooter(new_footer ? new_footer : "", new_line != 0);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t prependReportFooter(void *p_report, const char *new_footer, int32_t new_line)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->prependFooter(new_footer ? new_footer : "", new_line != 0);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    uint64_t getReportFooter(void *p_report, char *footer, uint64_t size)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = hebench::Utilities::copyString(footer, size, p->getFooter());
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t addEventType(void *p_report, uint32_t event_type_id, const char *event_type_header)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->newEventType(event_type_id, event_type_header ? event_type_header : "", false);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t addMainEventType(void *p_report, uint32_t event_type_id, const char *event_type_header)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->newEventType(event_type_id, event_type_header ? event_type_header : "", true);

            retval = 1; // success
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t hasEventType(void *p_report, uint32_t event_type_id)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = p->getEventTypes().count(event_type_id) > 0 ? 1 : 0;
        }
        catch (...)
        {
            retval = -1;
        }

        return retval;
    }

    uint64_t getEventTypeHeader(void *p_report, uint32_t event_type_id, char *event_type_header, uint64_t size)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            const auto &map_event_types = p->getEventTypes();
            if (map_event_types.count(event_type_id) <= 0)
                throw std::invalid_argument("ID not found.");

            retval = hebench::Utilities::copyString(event_type_header, size, map_event_types.at(event_type_id));
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    uint64_t getEventTypeCount(void *p_report)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = p->getEventTypes().size();
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t getMainEventType(void *p_report, uint32_t *p_event_type_id)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !p_event_type_id)
                throw std::invalid_argument("");

            if (p->getMainEventID() != std::numeric_limits<uint32_t>::max())
            {
                *p_event_type_id = p->getMainEventID();
                retval           = 1;
            } // end if
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t addEvent(void *p_report, const TimingReportEventC *p_event)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !p_event)
                throw std::invalid_argument("");

            std::shared_ptr<TimingReportEventC> sp_event = std::make_shared<TimingReportEventC>();
            if (!sp_event)
                throw std::bad_alloc();
            *sp_event = *p_event;
            p->newEvent(sp_event);

            retval = 1;
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t getEvent(void *p_report, TimingReportEventC *p_event, uint64_t index)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !p_event || index >= p->getEvents().size())
                throw std::invalid_argument("");

            *p_event = *p->getEvents()[index];

            retval = 1;
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    uint64_t getEventCount(void *p_report)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = p->getEvents().size();
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    uint64_t getEventCapacity(void *p_report)
    {
        uint64_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            retval = p->getEvents().capacity();
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t setEventCapacity(void *p_report, uint64_t new_capacity)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->reserveCapacityForEvents(new_capacity);

            retval = 1;
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t clearEvents(void *p_report)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p)
                throw std::invalid_argument("");

            p->clear();

            retval = 1;
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t save2CSV(void *p_report, const char *filename)
    {
        int32_t retval = 0;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !filename || filename[0] == '\0')
                throw std::invalid_argument("");

            std::ofstream fnum;
            fnum.open(filename, std::ios_base::out | std::ios_base::trunc);
            if (!fnum.is_open())
                throw std::ios_base::failure("Error opening file");

            p->convert2CSV(fnum);

            retval = 1;
        }
        catch (...)
        {
            retval = 0;
        }

        return retval;
    }

    int32_t convert2CSV(void *p_report, char **pp_csv_content)
    {
        int32_t retval      = 0;
        char *p_csv_content = nullptr;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !pp_csv_content)
                throw std::invalid_argument("");

            std::stringstream ss;
            std::string s_csv_content;

            p->convert2CSV(ss);
            s_csv_content = ss.str();
            ss            = std::stringstream();

            p_csv_content = new char[s_csv_content.length() + 1];
            if (!p_csv_content)
                throw std::bad_alloc();
            std::strcpy(p_csv_content, s_csv_content.c_str());
            *pp_csv_content = p_csv_content;

            retval = 1;
        }
        catch (...)
        {
            if (p_csv_content)
                delete[] p_csv_content;
            retval = 0;
        }

        return retval;
    }

    int32_t generateSummaryCSV(void *p_report, TimingReportEventSummaryC *p_main_event_summary, char **pp_csv_content)
    {
        int32_t retval      = 0;
        char *p_csv_content = nullptr;
        try
        {
            TimingReport *p = reinterpret_cast<TimingReport *>(p_report);
            if (!p || !p_main_event_summary || !pp_csv_content)
                throw std::invalid_argument("");

            std::stringstream ss;
            std::string s_csv_content;

            //ReportSummary::generateCSV(ss, *p_main_event_summary, *p);
            ReportSummary report_summary(*p);
            report_summary.generateCSV(ss);
            *p_main_event_summary = report_summary.getMainEventSummary();
            s_csv_content         = ss.str();
            ss                    = std::stringstream();

            p_csv_content = new char[s_csv_content.length() + 1];
            if (!p_csv_content)
                throw std::bad_alloc();
            std::strcpy(p_csv_content, s_csv_content.c_str());
            *pp_csv_content = p_csv_content;

            retval = 1;
        }
        catch (...)
        {
            if (p_csv_content)
                delete[] p_csv_content;
            retval = 0;
        }

        return retval;
    }

    void freeCSVContent(char *p_csv_content)
    {
        if (p_csv_content)
            delete[] p_csv_content;
    }

    void *loadReportFromCSV(const char *p_csv_content, char error_description[MAX_DESCRIPTION_BUFFER_SIZE])
    {
        TimingReport *p_retval = nullptr;
        try
        {
            p_retval = new TimingReport(TimingReport::loadCSV(p_csv_content));
        }
        catch (std::exception &ex)
        {
            if (p_retval)
            {
                delete p_retval;
                p_retval = nullptr;
            }
            hebench::Utilities::copyString(error_description, MAX_DESCRIPTION_BUFFER_SIZE, ex.what());
        }
        catch (...)
        {
            if (p_retval)
            {
                delete p_retval;
                p_retval = nullptr;
            }
            hebench::Utilities::copyString(error_description, MAX_DESCRIPTION_BUFFER_SIZE, "Unknown error.");
        }
        return p_retval;
    }

    void *loadReportFromCSVFile(const char *filename, char error_description[MAX_DESCRIPTION_BUFFER_SIZE])
    {
        TimingReport *p_retval = nullptr;
        try
        {
            std::ifstream fnum;
            if (!filename || filename[0] == '\0')
                throw std::invalid_argument("Invalid empty filename.");
            fnum.open(filename, std::ios_base::in);
            if (!fnum.is_open())
                throw std::ios_base::failure("Could not open file \"" + std::string(filename) + "\"");
            p_retval = new TimingReport(TimingReport::loadCSV(fnum));
        }
        catch (std::exception &ex)
        {
            if (p_retval)
            {
                delete p_retval;
                p_retval = nullptr;
            }
            hebench::Utilities::copyString(error_description, MAX_DESCRIPTION_BUFFER_SIZE, ex.what());
        }
        catch (...)
        {
            if (p_retval)
            {
                delete p_retval;
                p_retval = nullptr;
            }
            hebench::Utilities::copyString(error_description, MAX_DESCRIPTION_BUFFER_SIZE, "Unknown error.");
        }
        return p_retval;
    }
}

int32_t setTimingPrefix(TimingPrefixedSeconds *p_prefix, double seconds, char prefix)
{
    int32_t retval = 1;

    try
    {
        if (!retval)
            throw std::invalid_argument("p_prefix");
        TimingReport::setTimingPrefix(*p_prefix, seconds, prefix);
    }
    catch (...)
    {
        retval = 0;
    }

    return retval;
}

int32_t computeTimingPrefix(TimingPrefixedSeconds *p_prefix, double seconds)
{
    int32_t retval = 1;

    try
    {
        if (!retval)
            throw std::invalid_argument("p_prefix");
        TimingReport::computeTimingPrefix(*p_prefix, seconds);
    }
    catch (...)
    {
        retval = 0;
    }

    return retval;
}

} // namespace Report
} // namespace TestHarness
} // namespace hebench
