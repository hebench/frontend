// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <condition_variable>
#include <ctime>
#include <exception>
#include <ios>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <utility>

// headers to capture ctrl+c and seg fault on Linux.
#include <csignal>
#include <cstdlib>
#include <iostream>

#include "../include/logging.h"

using namespace hebench::Logging;

#define INTERNAL_LOG_LEVEL_MINIMAL  1
#define INTERNAL_LOG_LEVEL_BASIC    2
#define INTERNAL_LOG_LEVEL_ADVANCED 4
#define INTERNAL_LOG_LEVEL_VERBOSE  8

class ParallelLog
{
public:
    ParallelLog(std::size_t max_buffer_size, const std::string &filename, std::size_t max_size_bytes = 0);
    virtual ~ParallelLog();

    void queueOp(const std::string &operand);
    void flush();
    void waitForEmpty();

private:
    void doOp(const std::string &operand);

    // call only after locking mutex_op
    void checkLogSize();

    bool m_bstop;
    std::queue<std::string> m_queue;
    std::mutex m_mutex_queue;
    std::mutex m_mutex_op;
    std::condition_variable m_cv_enque;
    std::condition_variable m_cv_empty;
    std::thread m_worker;
    std::string m_filename;
    std::size_t m_max_file_size;
    std::size_t m_buffer_size;
    std::size_t m_max_buffer_size;

    std::chrono::system_clock::time_point m_start;
    std::fstream m_file_log;
};

ParallelLog::ParallelLog(std::size_t max_buffer_size, const std::string &filename, std::size_t max_size_bytes) :
    m_bstop(false), m_filename(filename), m_max_file_size(max_size_bytes), m_buffer_size(0), m_max_buffer_size(max_buffer_size)
{
    if (m_filename.empty())
        throw std::ios_base::failure("ParallelLog::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": invalid empty log file name.");

    m_file_log.open(m_filename, std::ofstream::in | std::ofstream::out | (m_max_file_size > 0 ? std::ofstream::trunc : std::ofstream::app));
    if (!m_file_log.is_open())
        throw std::ios_base::failure("ParallelLog::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": unable to open log file: \"" + m_filename + "\"");

    m_start           = std::chrono::system_clock::now();
    std::time_t t_now = std::chrono::system_clock::to_time_t(m_start);
    m_file_log << std::endl
               << "****************************************" << std::endl
               << std::ctime(&t_now) << std::endl;

    auto func = [this]() {
        while (!this->m_bstop)
        {
            std::unique_lock<std::mutex> lock_queue(m_mutex_queue);
            while (!this->m_bstop && this->m_queue.empty())
            {
                m_cv_empty.notify_all(); // notify anyone waiting that the
                    // queue is empty
                m_cv_enque.wait(lock_queue); // wait for enqueue
            }

            if (!this->m_bstop)
            {
                std::string operand = std::move(this->m_queue.front());
                this->m_queue.pop();

                lock_queue.unlock();

                doOp(operand);
            }
        }
    };

    m_worker = std::thread(func);
    queueOp(std::string()); // warm up
}

ParallelLog::~ParallelLog()
{
    // stop the thread
    {
        std::lock_guard<std::mutex> lock_queue(m_mutex_queue);
        this->m_bstop = true;
        m_cv_enque.notify_all(); // notify thread to stop
    } // lock release
    // wait for thread to stop
    if (m_worker.joinable())
        m_worker.join();

    std::lock_guard<std::mutex> lock_op(m_mutex_op); // wait for any op or flush

    // close file
    auto end                = std::chrono::system_clock::now();
    std::size_t diff_count  = std::chrono::duration_cast<std::chrono::hours>(end - m_start).count();
    std::string s_time_unit = "hours";
    if (diff_count <= 0)
    {
        diff_count  = std::chrono::duration_cast<std::chrono::minutes>(end - m_start).count();
        s_time_unit = "minutes";
    }
    if (diff_count <= 0)
    {
        diff_count  = std::chrono::duration_cast<std::chrono::seconds>(end - m_start).count();
        s_time_unit = "seconds";
    }
    if (diff_count <= 0)
    {
        diff_count  = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
        s_time_unit = "milliseconds";
    }
    std::time_t t_now = std::chrono::system_clock::to_time_t(end);
    m_file_log << std::endl
               << std::ctime(&t_now) << std::endl;
    m_file_log << "Elapsed log time: " << diff_count << " " << s_time_unit << std::endl;

    if (m_file_log.is_open())
        m_file_log.close();
}

void ParallelLog::queueOp(const std::string &operand)
{
    std::unique_lock<std::mutex> lock_op(m_mutex_op);
    if (m_buffer_size >= m_max_buffer_size)
    {
        // queue size is over max size, so,
        // wait for queue to empty before enqueuing again
        lock_op.unlock();
        waitForEmpty();
        lock_op.lock();
    }
    m_buffer_size += operand.size();
    lock_op.unlock();

    std::lock_guard<std::mutex> lock_queue(m_mutex_queue);
    m_queue.emplace(operand);
    m_cv_enque.notify_all(); // notify thread that there is data in the queue
    // unlock queue (auto)
}

void ParallelLog::flush()
{
    waitForEmpty();

    std::lock_guard<std::mutex> lock_op(m_mutex_op);

    if (m_file_log && m_file_log.is_open())
    {
        m_file_log.flush();
        checkLogSize();
    }
}

void ParallelLog::waitForEmpty()
{
    std::unique_lock<std::mutex> lock_queue(m_mutex_queue);
    while (!m_queue.empty())
        m_cv_empty.wait(lock_queue);
}

void ParallelLog::doOp(const std::string &operand)
{
    std::lock_guard<std::mutex> lock_op(m_mutex_op);

    m_buffer_size = (m_buffer_size < operand.size() ? 0 : m_buffer_size - operand.size());

    if (!operand.empty() && m_file_log && m_file_log.is_open())
    {
        m_file_log.write(operand.c_str(), operand.size());

        checkLogSize();
    }
}

void ParallelLog::checkLogSize()
{
    if (m_max_file_size > 0)
    {
        // maximum file size specified

        std::size_t size = (std::size_t)m_file_log.tellp();
        if (size > m_max_file_size)
        {
            // file exceeds maximum size

            // scroll all text from the rest of the file
            // to the begining to make space for the new data
            std::size_t new_pos = size - m_max_file_size + m_max_file_size / 3;
            std::vector<char> data_file;
            data_file.resize(size - new_pos);
            m_file_log.seekg(new_pos, std::ifstream::beg);
            m_file_log.read(data_file.data(), data_file.size());
            m_file_log.seekp(0, std::ofstream::beg); // rewind writing pointer
                // to begining of file
            m_file_log.write(data_file.data(), data_file.size());
            std::fstream::pos_type tmp_pos = m_file_log.tellp();
            // fill up excess data with blanks
            data_file         = std::vector<char>(80, ' ');
            data_file[0]      = '\n';
            std::size_t reps  = new_pos / data_file.size();
            std::size_t extra = new_pos % data_file.size();
            for (std::size_t i = 0; i < reps; i++)
                m_file_log.write(data_file.data(), data_file.size());
            if (extra > 0)
                m_file_log.write(data_file.data(), extra);
            m_file_log.flush();
            // set writing pointer to end of log
            m_file_log.seekp(tmp_pos, std::ofstream::beg);
        }
    }
}

//------------------------------------
// GlobalLogger class
//------------------------------------

bool GlobalLogger::OutputLogCount  = true;
bool GlobalLogger::OutputTimings   = true;
bool GlobalLogger::m_binitialized  = false;
LogLevel GlobalLogger::m_log_level = LogLevel::LogLevelNONE;
std::mutex GlobalLogger::m_mutex_logs;
std::unordered_map<std::thread::id, GlobalLogger::_InternalLogger> GlobalLogger::m_parallel_logs;
bool GlobalLogger::m_bmulti_threading                                = false;
bool GlobalLogger::m_binitialized_signal                             = false;
std::chrono::high_resolution_clock::time_point GlobalLogger::m_start = std::chrono::high_resolution_clock::now();
std::unordered_map<int, void (*)(int)> GlobalLogger::m_default_handlers;
std::size_t GlobalLogger::m_max_buffer_size = 0;
std::string GlobalLogger::m_log_file_name;
std::string GlobalLogger::m_log_file_ext;
std::size_t GlobalLogger::m_max_log_file_size = 0;

void GlobalLogger::initialize(LogLevel log_level, std::size_t max_buffer_size, const std::string &log_file_name, bool multi_threading,
                              std::size_t max_log_file_size)
{
    std::size_t found      = log_file_name.rfind(".");
    std::string s_filename = log_file_name;
    std::string s_ext;

    if (log_file_name.back() != '/' && log_file_name.back() != '\\')
    {
        if (found != std::string::npos)
        {
            s_ext      = log_file_name.substr(found + 1);
            s_filename = log_file_name.substr(0, found);
        } // end if
    } // end if

    initialize(log_level, max_buffer_size, s_filename, s_ext, multi_threading, max_log_file_size);
}

void GlobalLogger::initialize(LogLevel log_level, std::size_t max_buffer_size, const std::string &log_file_name,
                              const std::string &log_file_ext, bool multi_threading, std::size_t max_log_file_size)
{
    if (isInitialized())
        throw std::runtime_error("GlobalLogger::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": already initialized.");

    if (!m_binitialized_signal)
    {
        // capture all standard signals to flush the log before terminating
        m_default_handlers[SIGABRT] = std::signal(SIGABRT, criticalTerminate);
        m_default_handlers[SIGFPE]  = std::signal(SIGFPE, criticalTerminate);
        m_default_handlers[SIGILL]  = std::signal(SIGILL, criticalTerminate);
        m_default_handlers[SIGINT]  = std::signal(SIGINT, criticalTerminate);
        m_default_handlers[SIGSEGV] = std::signal(SIGSEGV, criticalTerminate);
        m_default_handlers[SIGTERM] = std::signal(SIGTERM, criticalTerminate);

        m_binitialized_signal = true;
    }

    m_log_file_name    = log_file_name;
    m_log_file_ext     = log_file_ext;
    m_bmulti_threading = multi_threading;

    if (max_log_file_size > 0 && max_buffer_size > max_log_file_size)
        throw std::runtime_error("Logging::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": file size cannot be smaller than buffer size.");
    m_max_log_file_size = max_log_file_size;

    if (max_buffer_size > 0)
        m_log_level = log_level;
    else
        m_log_level = LogLevelNONE;

    m_binitialized = true;
}

void GlobalLogger::terminate()
{
    if (isInitialized())
    {
        m_parallel_logs.clear();
        resetLogLevel();

        m_binitialized = false;
    }
}

std::string GlobalLogger::log(bool output_time_stamp, const char *message, bool bendl,
                              const char *function, const char *container, const char *filename,
                              int line_no, LogLevel required_log_level)
{
    // prepare log message
    std::string retval;
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
    retval = ss_retval.str();

    ss_retval = std::stringstream(); // clean up

    if (m_log_level != LogLevelNONE && (m_log_level & required_log_level))
    {
        if (!isInitialized())
            throw std::runtime_error(std::string(__FILE__) + ":GlobalLogger::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": not initialized.");

        _InternalLogger *plogger = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex_logs);
            if (m_parallel_logs.empty() || m_bmulti_threading)
            {
                if (m_parallel_logs.count(std::this_thread::get_id()) <= 0)
                {
                    // log for this thread does not exists
                    std::stringstream ss;
                    ss << m_log_file_name;
                    if (m_bmulti_threading) // put thread id in the file name
                        ss << "_" << std::this_thread::get_id();
                    if (!m_log_file_ext.empty())
                        ss << "." << m_log_file_ext;
                    ParallelLog *pplog = new ParallelLog(m_max_buffer_size, ss.str(), m_max_log_file_size);
                    if (!pplog)
                        throw std::runtime_error("Logging::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": allocation failed.");
                    _InternalLogger tmp_logger;
                    tmp_logger.m_log_cnt = 0;
                    tmp_logger.m_parallel_log.reset((void *)pplog, [](void *p) {
                        if (p)
                        {
                            ParallelLog *pp = reinterpret_cast<ParallelLog *>(p);
                            delete pp;
                        }
                    });
                    m_parallel_logs[std::this_thread::get_id()] = tmp_logger;
                }

                // logger for this thread already exists at this point
                plogger = &m_parallel_logs[std::this_thread::get_id()];
            }
            else
            {
                auto &pair_id_logger = *m_parallel_logs.begin();
                plogger              = &(pair_id_logger.second);
            }
        } // unlock

        if (plogger)
        {
            // count log
            ++plogger->m_log_cnt;

            // prepare log message
            std::stringstream ss;

            if (OutputLogCount)
                ss << plogger->m_log_cnt << ". ";
            if (!output_time_stamp && OutputTimings)
                ss << std::fixed << getTime<std::micro>() << ": ";
            ss << retval;
            if (bendl)
                ss << std::endl;

            // queue message to be logged
            ParallelLog *ppl = reinterpret_cast<ParallelLog *>(plogger->m_parallel_log.get());
            if (ppl)
                ppl->queueOp(ss.str());
        }
    }

    if (output_time_stamp)
    {
        ss_retval << std::fixed << getTime<std::micro>();
        if (!retval.empty())
            ss_retval << ": " << retval;
        retval = ss_retval.str();
    } // end if

    return retval;
}

std::string GlobalLogger::log(bool output_time_stamp, const std::string &message, bool bendl,
                              const std::string &function, const std::string &container,
                              const std::string &filename, int line_no, LogLevel required_log_level)

{
    return log(output_time_stamp, message.c_str(), bendl,
               function.empty() ? nullptr : function.c_str(), container.empty() ? nullptr : container.c_str(),
               filename.empty() ? nullptr : filename.c_str(), line_no, required_log_level);
}

void GlobalLogger::setLogLevel(LogLevel value)
{
    if (value != LogLevel::LogLevelNONE && !isInitialized())
        throw std::runtime_error(std::string(__FILE__) + ":GlobalLogger::" + std::string(__func__) + "():" + std::to_string(__LINE__) + ": not initialized.");
    m_log_level = value;
}

void GlobalLogger::flush()
{
    if (isInitialized())
    {
        // flush all the open logs
        for (auto pair_logger : m_parallel_logs)
        {
            _InternalLogger &logger = pair_logger.second;
            ParallelLog *ppl        = reinterpret_cast<ParallelLog *>(logger.m_parallel_log.get());
            if (ppl)
            {
                ppl->flush();
            }
        }
    }
}

extern "C" void GlobalLogger::criticalTerminate(int s)
{
    if (isInitialized())
    {
        flush();
    }

    std::cout << std::endl
              << "OS signal captured: " << s << std::endl;
    if (m_default_handlers.count(s) > 0)
        m_default_handlers[s](s);
    std::exit(s);
}
