
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_ReportCompiler_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_ReportCompiler_H_0596d40a3cce4b108a81595c50eb286d

#include "stddef.h"
#include "stdint.h"

extern "C"
{

    namespace hebench {
    namespace ReportGen {
    namespace Compiler {

    /**
 * @brief Configuration for a compiler run.
 */
    struct ReportCompilerConfigC
    {
        /**
     * @brief C-string containing the input file name.
     */
        const char *input_file;
        /**
     * @brief If non-zero, the compiled overview will be output to stdout.
     */
        int32_t b_show_overview;
        /**
     * @brief If non-zero, the run details will be omited. Any warning, error,
     * or important messages are directed to stderr.
     */
        int32_t b_silent;
        /**
     * @brief Fallback time unit when no time unit is specified for a specific output.
     * @details One of:
     * `0` - (default): an appropriate time unit will be used to keep the values between 1 and 1000.
     * `s` - seconds
     * `ms` - milliseconds
     * `us` - microseconds
     * `ns` - nanoseconds
     */
        char time_unit;
        /**
     * @brief Time unit for report statistics. If `0`, the fallback `time_unit` will be used.
     */
        char time_unit_stats;
        /**
     * @brief Time unit for report overview. If `0`, the fallback `time_unit` will be used.
     */
        char time_unit_overview;
        /**
     * @brief Time unit for report summaries. If `0`, the fallback `time_unit` will be used.
     */
        char time_unit_summary;
    };

    /**
 * @brief Runs the compiler using the specified configuration.
 * @param[in] p_config Configuration for the compiler execution.
 * @param[out] s_error Pointer to buffer to receive any error message. If `null`, error messages
 * are ignored.
 * @param[in] s_error_size Size, in bytes, of the memory pointed to by \p s_error. If zero, error messages
 * are ignored.
 * @return true on success.
 * @details If an error occurs, the error message will be saved in the provided buffer \p s_error.
 * This function will attempt to copy as many bytes from the error message into the buffer. Otherwise,
 * the contents of the buffer will remain unchanged. The buffer can `null` or the number of bytes
 * pointed to can be zero to indicate that caller is ignoring any error messages.
 */
    int32_t compile(const ReportCompilerConfigC *p_config, char *s_error, size_t s_error_size);

    } // namespace Compiler
    } // namespace ReportGen
    } // namespace hebench
}

#endif // defined _HEBench_ReportCompiler_H_0596d40a3cce4b108a81595c50eb286d
