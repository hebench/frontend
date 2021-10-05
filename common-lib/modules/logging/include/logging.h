// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _LOGGING_H_71409a03e9a84aa184b363b098e70fe9
#define _LOGGING_H_71409a03e9a84aa184b363b098e70fe9

#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define IL_DECLARE_CLASS_NAME(class_name) static constexpr const char *m_private_class_name = #class_name;

#define IL_LOG_MSG_CLASS(message) hebench::Logging::GlobalLogger::log((message),                            \
                                                                      true, __func__, m_private_class_name, \
                                                                      __FILE__, __LINE__)

#define IL_LOG_MSG_CLASS_LL(message, log_lvl) hebench::Logging::GlobalLogger::log((message),                            \
                                                                                  true, __func__, m_private_class_name, \
                                                                                  __FILE__, __LINE__, log_lvl)

#define IL_LOG_MSG(message) hebench::Logging::GlobalLogger::log((message),               \
                                                                true, __func__, nullptr, \
                                                                __FILE__, __LINE__)

#define IL_LOG_MSG_LL(message, log_lvl) hebench::Logging::GlobalLogger::log((message),               \
                                                                            true, __func__, nullptr, \
                                                                            __FILE__, __LINE__, log_lvl)

namespace hebench {
namespace Logging {

/**
 * @brief Specified the log level to be used by the GlobalLogger.
 */
enum LogLevel
{
    LogLevelNONE     = 0, /**< No logging. */
    LogLevelMINIMAL  = 1, /**< Minimal logging. */
    LogLevelBASIC    = 3, /**< Basic logging. */
    LogLevelADVANCED = 7, /**< Advanced logging. */
    LogLevelVERBOSE  = 15 /**< Verbose logging. */
};

/**
 * @brief Static class that provides global logging capabilities.
 *
 * @details This is a stand-alone class. It can be used to log anything from
 * an application non-specific to this library. If not initialized,
 * GlobalLogger::log() calls are ignored.
 */
class GlobalLogger
{
public:
    /**
     * @brief Initializes the GlobalLogger.
     * @param[in] log_level A member of LogLevel indicating the logging level.
     * @param[in] max_buffer_size Specifies the maximum memory buffer size, in
     * bytes, before logging gets flushed.
     * @param[in] log_file_name Name of the file, with extension, used to store
     * the log.
     * @param[in] multi_threading If true, every thread calling the log will
     * output to a specific file, otherwise, every thread will output to the
     * same file.
     * @param[in] max_log_file_size Maximum size, in bytes, for the log file.
     * Defaults to 0.
     *
     * @details If GlobalLogger is not initialized, calls to GlobalLogger::log()
     * will be ignored. This behavior is equivalent to LogLevel::LogLevelNONE.
     *
     * Only LogLevel::LogLevelNONE affects the behavior of calls to
     * GlobalLogger::log(). It is user's responsibility whether to support other
     * log levels and to check the current level to act accordingly before
     * logging.
     *
     * File name extension will be deducted from \p log_file_name.
     *
     * If multi_threading is true, every thread will be assigned a file to write
     * logs to of the form log_file_name + "_" + thread_id + "." + log_file_ext.
     * If multi_threading is false, all threads will write to the same log file
     * with name log_file_name + "." + log_file_ext.
     *
     * If \p max_log_file_size is 0, the log will be appended to the contents of
     * the file, if it exist, and the log file size will be limited only by
     * storage medium capacity. Otherwise, the log file will be overwritten and
     * the size will be kept to about as many bytes as requested by scrolling
     * existing logs. As a consequence, the ending of the log file may be padded
     * with blank lines and spaces to keep the file size constant.
     *
     * Every call to GlobalLogger::initialize() must have a matching call to
     * GlobalLogger::terminate() for the log to be properly closed.
     *
     * If \p log_file_name is invalid, \p max_buffer_size is zero, or
     * \p max_buffer_size is greater than \p max_log_file_size, a std::exception
     * is thrown.
     *
     * The first time this method is called, it will setup a signal capture to
     * capture the CTRL+C key combination to attempt to flush the log before
     * killing the calling process, thus avoiding loss of important log data
     * captured about errors, but still stored in temporary memory buffer.
     * IMPORTANT: this functionality is currently only supported in Linux based
     * OS.
     *
     * @sa \ref LogLevel GlobalLogger::terminate()
     */
    static void initialize(LogLevel log_level, std::size_t max_buffer_size,
                           const std::string &log_file_name, bool multi_threading,
                           std::size_t max_log_file_size = 0);
    /**
     * @brief Initializes the GlobalLogger.
     * @param[in] log_level A member of LogLevel indicating the logging level.
     * @param[in] max_buffer_size Specifies the maximum memory buffer size, in
     * bytes, before logging gets flushed.
     * @param[in] log_file_name Name of the file, without extension, used to store
     * the log.
     * @param[in] log_file_ext Extension of the file used to store the log.
     * @param[in] multi_threading If true, every thread calling the log will
     * output to a specific file, otherwise, every thread will output to the
     * same file.
     * @param[in] max_log_file_size Maximum size, in bytes, for the log file.
     * Defaults to 0.
     *
     * @details If GlobalLogger is not initialized, calls to GlobalLogger::log()
     * will be ignored. This behavior is equivalent to LogLevel::LogLevelNONE.
     *
     * Only LogLevel::LogLevelNONE affects the behavior of calls to
     * GlobalLogger::log(). It is user's responsibility whether to support other
     * log levels and to check the current level to act accordingly before
     * logging.
     *
     * If multi_threading is true, every thread will be assigned a file to write
     * logs to of the form \p log_file_name + "_" + thread_id + "." + \p log_file_ext.
     * If multi_threading is false, all threads will write to the same log file
     * with name \p log_file_name + "." + \p log_file_ext.
     *
     * If \p max_log_file_size is 0, the log will be appended to the contents of
     * the file, if it exist, and the log file size will be limited only by
     * storage medium capacity. Otherwise, the log file will be overwritten and
     * the size will be kept to about as many bytes as requested by scrolling
     * existing logs. As a consequence, the ending of the log file may be padded
     * with blank lines and spaces to keep the file size constant.
     *
     * Every call to GlobalLogger::initialize() must have a matching call to
     * GlobalLogger::terminate() for the log to be properly closed.
     *
     * If \p log_file_name is invalid, \p max_buffer_size is zero, or
     * \p max_buffer_size is greater than \p max_log_file_size, a std::exception
     * is thrown.
     *
     * The first time this method is called, it will setup a signal capture to
     * capture the CTRL+C key combination to attempt to flush the log before
     * killing the calling process, thus avoiding loss of important log data
     * captured about errors, but still stored in temporary memory buffer.
     * IMPORTANT: this functionality is currently only supported in Linux based
     * OS.
     *
     * @sa \ref LogLevel GlobalLogger::terminate()
     */
    static void initialize(LogLevel log_level, std::size_t max_buffer_size,
                           const std::string &log_file_name, const std::string &log_file_ext,
                           bool multi_threading, std::size_t max_log_file_size = 0);
    /**
     * @brief Terminates the log, flushing any pending log requests and closing
     * the log file.
     *
     * A call to GlobalLogger::initialize() is needed after a call to
     * terminate() in order to start logging again.
     *
     * Every call to GlobalLogger::initialize() must have a matching call to
     * GlobalLogger::terminate() for the log to be properly closed.
     */
    static void terminate();

    /**
     * @brief Adds the specified message to the log.
     * @param[in] message Message to output to the log.
     * @param[in] bendl If true, a new line is added at the end of the logged
     * message.
     * @param[in] function Function where call to log occurred. Can be null.
     * @param[in] container Container of the function (class, namespace, etc.).
     * Can be null.
     * @param[in] filename Name of the file where call to log occurred. Can be
     * null.
     * @param[in] line_no Number of the line in the file where call to log
     * occurred.
     * @param[in] required_log_level Minimum log level required for the message
     * to be appended to the log.
     *
     * @details If the current log level is lower than \p required_log_level,
     * then, this method does nothing, otherwise, this method will construct a
     * message of the form:
     *
     * [count. ][\p filename:][\p container][::\p function][:\p line_no][: ]\p message.
     *
     * The log message will be appended to the log asynchronously. If log buffer
     * size is greater than specified during initialization, this call will
     * stall until the log is flushed instead.
     *
     * Null parameters will be ignored.
     * If \p line_no is negative, it will be ignored.
     *
     * Count will be output if GlobalLogger::OutputLogCount is true.
     *
     * A \p required_log_level of LogLevelNONE will be ignored and will have the
     * same effect as if the log level is below \p required_log_level.
     *
     * @sa GlobalLogger::OutputLogCount
     */
    static std::string log(const char *message, bool bendl = true,
                           const char *function = nullptr, const char *container = nullptr,
                           const char *filename = nullptr, int line_no = -1,
                           LogLevel required_log_level = LogLevel::LogLevelMINIMAL)
    {
        return log(false, message, bendl, function, container, filename, line_no, required_log_level);
    }
    /**
     * @brief Adds the specified message to the log.
     * @param[in] message Message to output to the log.
     * @param[in] bendl If true, a new line is added at the end of the logged
     * message.
     * @param[in] function Function where call to log occurred. Can be empty.
     * @param[in] container Container of the function (class, namespace, etc.).
     * Can be empty.
     * @param[in] filename Name of the file where call to log occurred. Can be
     * empty.
     * @param[in] line_no Number of the line in the file where call to log
     * occurred.
     * @param[in] required_log_level Minimum log level required for the message
     * to be appended to the log.
     *
     * @details If the current log level is lower than \p required_log_level,
     * then, this method does nothing, otherwise, this method will construct a
     * message of the form:
     *
     * [count. ][\p filename:][\p container][::\p function][:\p line_no][: ]\p message.
     *
     * The log message will be appended to the log asynchronously. If log buffer
     * size is greater than specified during initialization, this call will
     * stall until the log is flushed instead.
     *
     * Null parameters will be ignored.
     * If \p line_no is negative, it will be ignored.
     *
     * Count will be output if GlobalLogger::OutputLogCount is true.
     *
     * A \p required_log_level of LogLevelNONE will be ignored and will have the
     * same effect as if the log level is below \p required_log_level.
     *
     * @sa GlobalLogger::OutputLogCount
     */
    static std::string log(const std::string &message, bool bendl = true, const std::string &function = std::string(),
                           const std::string &container = std::string(), const std::string &filename = std::string(), int line_no = -1,
                           LogLevel required_log_level = LogLevel::LogLevelMINIMAL)
    {
        return log(false, message, bendl, function, container, filename, line_no, required_log_level);
    }
    /**
     * @brief Adds the specified message to the log.
     * @param[in] output_time_stamp If true, time stamp is output as part of the
     * logged message.
     * @param[in] message Message to output to the log.
     * @param[in] bendl If true, a new line is added at the end of the logged
     * message.
     * @param[in] function Function where call to log occurred. Can be null.
     * @param[in] container Container of the function (class, namespace, etc.).
     * Can be null.
     * @param[in] filename Name of the file where call to log occurred. Can be
     * null.
     * @param[in] line_no Number of the line in the file where call to log
     * occurred.
     * @param[in] required_log_level Minimum log level required for the message
     * to be appended to the log.
     *
     * @details If the current log level is lower than \p required_log_level,
     * then, this method does nothing, otherwise, this method will construct a
     * message of the form:
     *
     * [count. ][time_stamp: ][\p filename:][\p container][::\p function][:\p line_no][: ]\p message.
     *
     * The log message will be appended to the log asynchronously. If log buffer
     * size is greater than specified during initialization, this call will
     * stall until the log is flushed instead.
     *
     * `GlobalLogger::OutputTimings` controls whether time stamp will appear on the
     * logged message. Parameter \p output_time_stamp controls whether time stamp
     * will appear on the returned string message of this method.
     *
     * Null parameters will be ignored.
     * If \p line_no is negative, it will be ignored.
     *
     * Count will be output if GlobalLogger::OutputLogCount is true.
     *
     * A \p required_log_level of LogLevelNONE will be ignored and will have the
     * same effect as if the log level is below \p required_log_level.
     *
     * @sa GlobalLogger::OutputLogCount, GlobalLogger::OutputTimings
     */
    static std::string log(bool output_time_stamp, const char *message, bool bendl = true,
                           const char *function = nullptr, const char *container = nullptr,
                           const char *filename = nullptr, int line_no = -1,
                           LogLevel required_log_level = LogLevel::LogLevelMINIMAL);
    /**
     * @brief Adds the specified message to the log.
     * @param[in] output_time_stamp If true, time stamp is output as part of the
     * logged message.
     * @param[in] message Message to output to the log.
     * @param[in] bendl If true, a new line is added at the end of the logged
     * message.
     * @param[in] function Function where call to log occurred. Can be empty.
     * @param[in] container Container of the function (class, namespace, etc.).
     * Can be empty.
     * @param[in] filename Name of the file where call to log occurred. Can be
     * empty.
     * @param[in] line_no Number of the line in the file where call to log
     * occurred.
     * @param[in] required_log_level Minimum log level required for the message
     * to be appended to the log.
     *
     * @details If the current log level is lower than \p required_log_level,
     * then, this method does nothing, otherwise, this method will construct a
     * message of the form:
     *
     * [count. ][time_stamp: ][\p filename:][\p container][::\p function][:\p line_no][: ]\p message.
     *
     * The log message will be appended to the log asynchronously. If log buffer
     * size is greater than specified during initialization, this call will
     * stall until the log is flushed instead.
     *
     * `GlobalLogger::OutputTimings` controls whether time stamp will appear on the
     * logged message. Parameter \p output_time_stamp controls whether time stamp
     * will appear on the returned string message of this method.
     *
     * Empty strings will be ignored.
     * If \p line_no is negative, it will be ignored.
     *
     * Count will be output if GlobalLogger::OutputLogCount is true.
     *
     * A \p required_log_level of LogLevelNONE will be ignored and will have the
     * same effect as if the log level is below \p required_log_level.
     *
     * @sa GlobalLogger::OutputLogCount, GlobalLogger::OutputTimings
     */
    static std::string log(bool output_time_stamp, const std::string &message, bool bendl = true,
                           const std::string &function = std::string(), const std::string &container = std::string(),
                           const std::string &filename = std::string(), int line_no = -1,
                           LogLevel required_log_level = LogLevel::LogLevelMINIMAL);

    /**
     * @brief Retrieves whether GlobalLogger is initialized.
     * @return true if GlobalLogger is initialized, false otherwise.
     */
    static bool isInitialized() { return m_binitialized; }

    /**
     * @brief Retrieves the current log level.
     *
     * @sa GlobalLogger::setLogLevel()
     */
    static LogLevel getLogLevel() { return m_log_level; }

    /**
     * @brief Sets the current log level.
     * @param[in] value New log level to set.
     *
     * @details Throws std::runtime_error if value is not LogLevelNONE and
     * GlobalLogger is not initialized.
     *
     * @sa GlobalLogger::setLogLevel() GlobalLogger::isInitialized()
     */
    static void setLogLevel(LogLevel value);

    /**
     * @brief Sets the current log level to LogLevel::LogLevelNONE.
     */
    static void resetLogLevel() { setLogLevel(LogLevel::LogLevelNONE); }

    /**
     * @brief Flushes any pending log messages to the log file.
     */
    static void flush();

    /**
     * @brief Specifies whether a count will be output for each log message.
     *
     * @details If true, every call to GlobalLogger::log() will prepend the
     * current log count to the message. Otherwise, no count will be added.
     */
    static bool OutputLogCount;
    /**
     * @brief Specifies whether the log timong will be output for each log
     * message.
     *
     * @details If true, every call to GlobalLogger::log() will prepend the
     * time in microseconds since the current log started to the message.
     * Otherwise, no timing will be added.
     */
    static bool OutputTimings;

private:
    struct _InternalLogger
    {
        _InternalLogger() :
            m_log_cnt(0) {}
        _InternalLogger(const _InternalLogger &src) :
            m_log_cnt(src.m_log_cnt), m_parallel_log(src.m_parallel_log) {}
        _InternalLogger &operator=(const _InternalLogger &src)
        {
            if (this != &src)
            {
                m_log_cnt      = src.m_log_cnt;
                m_parallel_log = src.m_parallel_log;
            }
            return *this;
        }
        std::size_t m_log_cnt;
        std::shared_ptr<void> m_parallel_log;
    };

    // TimeInterval = std::nano, std::micro, std::milli, etc.
    template <class TimeInterval>
    static double getTime()
    {
        return std::chrono::duration<double, TimeInterval>(std::chrono::high_resolution_clock::now() - m_start).count();
    }
    static void criticalTerminate(int s);
    static std::unordered_map<int, void (*)(int)> m_default_handlers;

    static bool m_binitialized;
    static LogLevel m_log_level;
    static std::mutex m_mutex_logs;
    static std::unordered_map<std::thread::id, _InternalLogger> m_parallel_logs;
    static bool m_bmulti_threading;

    static std::size_t m_max_buffer_size;
    static std::string m_log_file_name;
    static std::string m_log_file_ext;
    static std::size_t m_max_log_file_size;

    static std::chrono::high_resolution_clock::time_point m_start;

    static bool m_binitialized_signal;
};

} // namespace Logging
} // namespace hebench

#endif // _LOGGING_H_71409a03e9a84aa184b363b098e70fe9
