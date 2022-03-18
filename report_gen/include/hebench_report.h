
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Report_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Report_H_0596d40a3cce4b108a81595c50eb286d

#include <stdint.h>

#include "hebench_report_types.h"

namespace hebench {
namespace TestHarness {
namespace Report {

extern "C"
{

    void *allocateReport();
    void freeReport(void *p_report);

    int32_t setReportHeader(void *p_report, const char *new_header);
    /**
     * @brief Appends text to existing header.
     * @param p_report
     * @param new_header
     * @param new_line If true a new line will be added after current header before
     * appending \p new_header .
     * @returns `true` on success
     */
    int32_t appendReportHeader(void *p_report, const char *new_header, int32_t new_line);
    /**
     * @brief Prepends text to existing header.
     * @param p_report
     * @param new_header
     * @param new_line If true, \p new_header , followed by a new line, will be prepended
     * to the current header. Otherwise, the current header will follow \p new_header
     * immediately without a new line.
     * @returns `true` on success
     */
    int32_t prependReportHeader(void *p_report, const char *new_header, int32_t new_line);
    uint64_t getReportHeader(void *p_report, char *header, uint64_t size);

    int32_t setReportFooter(void *p_report, const char *new_footer);
    /**
     * @brief Appends text to existing footer.
     * @param p_report
     * @param new_footer
     * @param new_line If true a new line will be added after current footer before
     * appending \p new_footer .
     * @returns `true` on success
     */
    int32_t appendReportFooter(void *p_report, const char *new_footer, int32_t new_line);
    /**
     * @brief Prepends text to existing footer.
     * @param p_report
     * @param new_footer
     * @param new_line If true, \p new_footer , followed by a new line, will be prepended
     * to the current footer. Otherwise, the current footer will follow \p new_footer
     * immediately without a new line.
     * @returns `true` on success
     */
    int32_t prependReportFooter(void *p_report, const char *new_footer, int32_t new_line);
    uint64_t getReportFooter(void *p_report, char *footer, uint64_t size);

    // event type: groups a collection of events under the same type

    /**
     * @brief addEventType
     * @param p_report
     * @param event_type_id
     * @param event_type_header
     * @returns `true` on success.
     */
    int32_t addEventType(void *p_report, uint32_t event_type_id, const char *event_type_header);
    /**
     * @brief Adds an event type and marks it as the main event.
     * @param p_report
     * @param event_type_id
     * @param event_type_header
     * @returns `true` on success.
     * @details
     * The main event is the event on which the summary will focus.
     */
    int32_t addMainEventType(void *p_report, uint32_t event_type_id, const char *event_type_header);
    /**
     * @brief hasEventType
     * @param p_report
     * @param event_type_id
     * @return Less than 0 on error, greater than 0 if report has an
     * event of the specified ID type, 0 otherwise.
     */
    int32_t hasEventType(void *p_report, uint32_t event_type_id);
    uint64_t getEventTypeHeader(void *p_report, uint32_t event_type_id, char *event_type_header, uint64_t size);
    uint64_t getEventTypeCount(void *p_report);
    /**
     * @brief getMainEventType
     * @param p_report
     * @param p_event_type_id
     * @returns `true` if report has main event type and it was successfully retrieved
     * into \p p_event_type_id .
     * @returns `false` on error or otherwise.
     */
    int32_t getMainEventType(void *p_report, uint32_t *p_event_type_id);

    // events management

    /**
     * @brief addEvent
     * @param p_report
     * @param p_event
     * @returns `true` on success.
     */
    int32_t addEvent(void *p_report, const TimingReportEventC *p_event);
    /**
     * @brief getEvent
     * @param p_report
     * @param p_event
     * @param index
     * @returns `true` on success.
     */
    int32_t getEvent(void *p_report, TimingReportEventC *p_event, uint64_t index);
    uint64_t getEventCount(void *p_report);
    uint64_t getEventCapacity(void *p_report);
    /**
     * @brief setEventCapacity
     * @param p_report
     * @param new_capacity
     * @returns `true` on success.
     */
    int32_t setEventCapacity(void *p_report, uint64_t new_capacity);
    int32_t clearEvents(void *p_report);

    // CSV
    int32_t save2CSV(void *p_report, const char *filename);
    /**
     * @brief convert2CSV
     * @param p_report
     * @param[out] pp_csv_content Deallocate using `freeCSVContent()`
     * @returns `true` on success.
     */
    int32_t convert2CSV(void *p_report, char **pp_csv_content);
    /**
     * @brief generateSummaryCSV
     * @param p_report
     * @param[out] p_main_event_summary TimingReportEventSummaryC struct where to store the
     * summary of the main event.
     * @param[out] pp_csv_content Deallocate using `freeCSVContent()`
     * @returns `true` on success.
     */
    int32_t generateSummaryCSV(void *p_report, TimingReportEventSummaryC *p_main_event_summary, char **pp_csv_content);
    /**
     * @brief Releases resources allocated by functions that generate CSV
     * formatted reports from a timing report.
     * @param[in] p_csv_content Pointer to the string buffer allocated by
     * functions that generate CSV reports.
     * @details Use this function to free resources allocated by
     * `convert2CSV()`, `generateSummaryCSV()`, and similar functions.
     */
    void freeCSVContent(char *p_csv_content);

    void *loadReportFromCSV(const char *p_csv_content, char error_description[MAX_DESCRIPTION_BUFFER_SIZE]);
    void *loadReportFromCSVFile(const char *filename, char error_description[MAX_DESCRIPTION_BUFFER_SIZE]);

    // misc/utilities

    /**
     * @brief Computes the value for the time unit based on a specified prefix.
     * @param[out] p_prefix Structure where to store the result.
     * @param[in] seconds Time in seconds for which to compute prefix.
     * @param[in] prefix Timing prefix specification. See details.
     * @details
     * Given a timing in seconds and the metric prefix, this function will compute
     * the corresponding value.
     *
     * Values for \p prefix are:
     *
     * 0: behaves as computeTimingPrefix()
     * `'s'`: result is in seconds.
     * `'m'`: result is in milliseconds.
     * `'u'`: result is in microseconds.
     * `'n'`: result is in nanoseconds.
     *
     * Any other value makes the function return a failure.
     *
     * For example, if \p seconds is `0.05` and \p prefix is `m`, then, the result is:
     *
     * @code
     * p_prefix->value                   = 50;
     * p_prefix->time_interval_ratio_den = 1000;
     * p_prefix->symbol                  = 'm';
     * p_prefix->prefix                  = 'milli';
     * @endcode
     */
    int32_t setTimingPrefix(TimingPrefixedSeconds *p_prefix, double seconds, char prefix);

    /**
     * @brief Retrieves the prefix for the time unit.
     * @param[out] p_prefix Structure where to store the result.
     * @param[in] seconds Time in seconds for which to compute prefix.
     * @details
     * Given a timing in seconds, this function will compute the metric prefix.
     * The time used are:
     * greater than 1: seconds
     * between 1 and 0.001: milliseconds
     * between 0.001 and 0.000001: microseconds
     * between 0.000001 and 0.000000001: nanoseconds
     */
    int32_t computeTimingPrefix(TimingPrefixedSeconds *p_prefix, double seconds);
}

} // namespace Report
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Report_H_0596d40a3cce4b108a81595c50eb286d
