
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Utilities_SRC_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Utilities_SRC_0596d40a3cce4b108a81595c50eb286d

#include <algorithm>

#include "../hebench_utilities_harness.h"
#include "modules/general/include/hebench_utilities.h"

namespace hebench {
namespace Utilities {

template <typename T>
void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          bool output_row_index,
                          const char *separator)
{
    std::uint64_t max_rows = 0;
    if (p_buffers && count > 0)
    {
        auto it  = std::max_element(p_buffers, p_buffers + count,
                                   [](const hebench::APIBridge::NativeDataBuffer *lhs, const hebench::APIBridge::NativeDataBuffer *rhs) {
                                       return lhs->size < rhs->size;
                                   });
        max_rows = (*it)->size / sizeof(T);
    } // end if

    for (std::uint64_t row_i = 0; row_i < max_rows; ++row_i)
    {
        // print next row
        if (output_row_index)
            os << row_i << separator;
        for (std::size_t buffer_i = 0; buffer_i < count; ++buffer_i)
        {
            if (buffer_i > 0)
                os << separator;
            if (p_buffers[buffer_i]->p && row_i < p_buffers[buffer_i]->size / sizeof(T))
                os << reinterpret_cast<T *>(p_buffers[buffer_i]->p)[row_i];
        } // end for
        os << std::endl;
    } // end for
}

template <class TimeInterval>
inline void TimingReportEx::addEvent(hebench::Common::TimingReportEvent::Ptr p_event)
{
    this->addEvent<TimeInterval>(p_event, nullptr);
}

template <class TimeInterval>
inline void TimingReportEx::addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                                     const std::string &event_type_name)
{
    this->addEvent<TimeInterval>(p_event, event_type_name.c_str());
}

template <class TimeInterval>
inline void TimingReportEx::addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                                     const char *event_type_name)
{
    if (event_type_name)
        this->addEventType(p_event->id, event_type_name);
    hebench::TestHarness::Report::TimingReportEventC tre_c =
        convert2C<TimeInterval>(*p_event);
    this->addEvent(tre_c);
}

template <class TimeInterval>
inline hebench::TestHarness::Report::TimingReportEventC
TimingReportEx::convert2C(const hebench::Common::TimingReportEvent &timing_event)
{
    hebench::TestHarness::Report::TimingReportEventC retval;
    retval.time_interval_ratio_num = TimeInterval::num;
    retval.time_interval_ratio_den = TimeInterval::den;
    retval.event_type_id           = timing_event.id;
    retval.cpu_time_start          = timing_event.timeStartCPU<TimeInterval>();
    retval.cpu_time_end            = timing_event.timeEndCPU<TimeInterval>();
    retval.wall_time_start         = timing_event.timeStartWall<TimeInterval>();
    retval.wall_time_end           = timing_event.timeEndWall<TimeInterval>();
    retval.iterations              = timing_event.iterations();
    copyString(retval.description, MAX_TIME_REPORT_EVENT_DESCRIPTION_SIZE, timing_event.description);

    return retval;
}

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Harness_Utilities_SRC_0596d40a3cce4b108a81595c50eb286d
