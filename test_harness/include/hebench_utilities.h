
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d

#include <filesystem>
#include <functional>
#include <ostream>
#include <random>
#include <string>

#include "hebench/api_bridge/types.h"
#include "hebench_report_cpp.h"
#include "modules/timer/include/timer.h"

namespace hebench {
namespace Utilities {

class UtilityPath
{
private:
    std::stringstream _ss_path;
    std::filesystem::path _fs_path;
    bool _is_single_path_dir;
    /**
    * @brief Helper function, in case is_single_path_dir is set to true, updates the fs_path
    * to leverage from the std::filesystem and std::filesystem::path functions.
    */
    void update();

public:
    /**
     * @brief UtilityPath Constructor
     * @param[in] is_single_path_dir allows the class to use a single path file or not.
     * @details if \p is_single_path_dir is true, then a std::stringstream will be used to
     * handle the operations, otherwise, a std::filesystem::path will handle the
     * functionality.
     */
    UtilityPath(bool is_single_path_dir);
    /**
     * @brief Getter for \p is_single_path_dir
     */
    bool is_single_path_dir();
    /**
     * @brief Converts the object to a std::filesystem::path
     */
    operator std::filesystem::path();
    /**
     * @brief Converts the object to a std::string
     */
    operator std::string();
    /**
     * @brief Copy constructor
     * @param[in] rhs will be copied to the lhs object.
     */
    UtilityPath &operator=(UtilityPath &rhs);
    /**
     * @brief Copy constructor
     * @param[in] rhs will be copied to the lhs object.
     */
    UtilityPath &operator=(const std::string &rhs);
    /**
     * @brief To append a new value to the UtilityPath.
     * @param[in] rhs will be appended to the lhs object.
     */
    void operator/=(const std::string &rhs);
    /**
     * @brief To add a new value to the UtilityPath.
     * @param[in] rhs will be added to the lhs object.
     */
    void operator+=(const std::string &rhs);
    /**
     * @brief To check if UtilityPath is absolute.
     */
    bool is_absolute();
    /**
     * @brief To check if UtilityPath exists.
     */
    bool exists();
    /**
     * @brief To check if UtilityPath is a regular file.
     */
    bool is_regular_file();
    /**
     * @brief To create the directories the UtilityPath holds.
     * @details If \p is_single_path_dir is true, no directory
     * will be created, since is set to be used as a single file
     * path.
     */
    bool create_directories();
    /**
     * @brief To remove the file that the UtilityPath holds.
     */
    int remove();
};

// Overloading operators to be used with different objects and mantain most of the original code.
std::ostream &operator<<(std::ostream &output, UtilityPath &report_path);
std::string operator/(std::string &lhs, UtilityPath &rhs);
std::string operator/(const std::string &lhs, UtilityPath &rhs);
std::string operator/(std::filesystem::path &lhs, UtilityPath &rhs);

/**
 * @brief Converts a string to directory name friendly.
 * @param[in] s String to convert.
 * @param[in] to_lowercase Specifies whether to return all lowercase.
 * @return A string that can be used as a directory name.
 * @details This function will return a string generated from \p s where the result
 * is composed only of alpha-numeric characters and non-repeating underscores.
 * The result will have no leading or trailing underscores. If \p to_lowercase flag
 * is true, all characters will lowercase, otherwise, they will keep the case originally
 * encountered in \p s. Non-alpha-numeric characters will be turned into underscores.
 *
 * For example:
 * @code
 * s = "Hello, World!"
 *
 * std::cout << convertToDirectoryName(s, true);
 *
 * // Prints:
 * // hello_world
 * @endcode
 */
std::string convertToDirectoryName(const std::string &s, bool to_lowercase = true);
std::uint64_t copyString(char *dst, std::uint64_t size, const std::string &src);
void writeToFile(const std::string &filename, std::function<void(std::ostream &)> fn,
                 bool b_binary, bool b_append = false);
void writeToFile(const std::string &filename,
                 const char *p_data, std::size_t size,
                 bool b_binary, bool b_append = false);

/**
 * @brief Writes the collection of `NativeDataBuffer` as columns to the specified
 * output stream.
 * @param os Stream where to write.
 * @param[in] p_buffers Array of `NativeDataBuffer` where each element will be output
 * per column. Each element does not need to have the same number of rows.
 * @param[in] count Number of elements in array \p p_buffers.
 * @param[in] data_type Type of data pointed to by \p p_buffers ' `NativeDataBuffer`.
 * @param[in] separator Separator text to use when separating the columns.
 * @details The number of elements in each buffer pointed by \p p_buffers does not
 * need to match. This method will print as many elements as each buffer has per row,
 * so buffers with fewer elements will be skipped in the output after all their elements
 * have been printed out.
 */
void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          hebench::APIBridge::DataType data_type,
                          bool output_row_index = false,
                          const char *separator = " ");
template <typename T>
/**
 * @brief Writes the collection of `NativeDataBuffer` as columns to the specified
 * output stream.
 * @param os Stream where to write.
 * @param[in] p_buffers Array of `NativeDataBuffer` where each element will be output
 * per column. Each element does not need to have the same number of rows. It is assumed
 * that buffers in this array point to data of type `T`.
 * @param[in] count Number of elements in array \p p_buffers.
 * @param[in] separator Separator text to use when separating the columns.
 * @details The number of elements in each buffer pointed by \p p_buffers does not
 * need to match. This method will print as many elements as each buffer has per row,
 * so buffers with fewer elements will be skipped in the output after all their elements
 * have been printed out.
 */
void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          bool output_row_index = false,
                          const char *separator = " ");

class RandomGenerator
{
private:
    static std::mt19937 m_rand;

public:
    static decltype(m_rand) &get() { return m_rand; }
    static void setRandomSeed(std::uint64_t seed);
    static void setRandomSeed();
};

class TimingReportEx : public hebench::TestHarness::Report::cpp::TimingReport
{
private:
    using Base = hebench::TestHarness::Report::cpp::TimingReport;

public:
    TimingReportEx(const std::string &header = std::string()) :
        TimingReport(header) {}
    ~TimingReportEx() override {}

    using Base::addEvent;
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event);
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                  const std::string &event_type_name);
    //std::string generateSummaryCSV(TimingReportEventC &main_event_summary);

    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    static hebench::TestHarness::Report::TimingReportEventC convert2C(const hebench::Common::TimingReportEvent &timing_event);

private:
    template <class TimeInterval> // TimeInterval must be a std::ratio<num, den>
    void addEvent(hebench::Common::TimingReportEvent::Ptr p_event,
                  const char *event_type_name);
};

} // namespace Utilities
} // namespace hebench

#include "inl/hebench_utilities.inl"

#endif // defined _HEBench_Harness_Utilities_H_0596d40a3cce4b108a81595c50eb286d
