
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "include/hebench_utilities.h"

namespace hebench {
namespace Utilities {

std::mt19937 RandomGenerator::m_rand;

std::string convertToDirectoryName(const std::string &s, bool to_lowercase)
{
    std::stringstream ss_retval;
    std::string s_tmp = s;

    // convert all non-alnum to underscores and, if requested, alnum to lowercase
    std::transform(s_tmp.begin(), s_tmp.end(), s_tmp.begin(),
                   [to_lowercase](unsigned char c) { return (std::isalnum(c) || c == '.' ?
                                                                 (to_lowercase ? std::tolower(c) : c) :
                                                                 '_'); });
    // remove leading and trailing underscores
    s_tmp.erase(0, s_tmp.find_first_not_of('_'));
    s_tmp.erase(s_tmp.find_last_not_of('_') + 1);

    // remove contiguous underscore repetitions
    std::size_t pos = 0;
    while (pos < s_tmp.size())
    {
        std::size_t i = s_tmp.find_first_of('_', pos);
        if (i == std::string::npos)
            ss_retval << s_tmp.substr(pos);
        else
        {
            if (i < s_tmp.size())
                ++i;
            ss_retval << s_tmp.substr(pos, i - pos);
            i = s_tmp.find_first_not_of('_', i);
        } // end if
        pos = i;
    } // end while

    return ss_retval.str();
}

UtilityPath::UtilityPath(bool is_single_path_dir) :
    _is_single_path_dir(is_single_path_dir) {}

bool UtilityPath::is_single_path_dir()
{
    return this->_is_single_path_dir;
}

inline void UtilityPath::update()
{
    if (this->_is_single_path_dir)
    {
        this->_fs_path = this->_ss_path.str();
    }
}

UtilityPath::operator std::string()
{
    return this->_is_single_path_dir ? this->_ss_path.str() : std::string(this->_fs_path);
}

UtilityPath::operator std::filesystem::path()
{
    this->update();
    return this->_fs_path;
}

UtilityPath &UtilityPath::operator=(UtilityPath &rhs)
{
    this->_is_single_path_dir = rhs.is_single_path_dir();
    if (this->_is_single_path_dir)
    {
        this->_ss_path.str("");
        this->_ss_path.clear();
        this->_ss_path << std::string(rhs);
    }
    else
    {
        this->_fs_path = std::string(rhs);
    }
    return *this;
}

UtilityPath &UtilityPath::operator=(const std::string &rhs)
{
    if (this->_is_single_path_dir)
    {
        this->_ss_path.str("");
        this->_ss_path.clear();
        this->_ss_path << rhs;
    }
    else
    {
        this->_fs_path = rhs;
    }
    return *this;
}

void UtilityPath::operator/=(const std::string &rhs)
{
    if (this->_is_single_path_dir)
    {
        if (!this->_ss_path.str().empty())
        {
            this->_ss_path << "-";
        }
        this->_ss_path << rhs;
    }
    else
    {
        this->_fs_path /= rhs;
    }
}

void UtilityPath::operator+=(const std::string &rhs)
{
    if (this->_is_single_path_dir)
    {
        this->_ss_path << rhs;
    }
    else
    {
        this->_fs_path += rhs;
    }
}

bool UtilityPath::is_absolute()
{
    this->update();
    return this->_fs_path.is_absolute();
}

bool UtilityPath::exists()
{
    this->update();
    return std::filesystem::exists(this->_fs_path);
}

bool UtilityPath::is_regular_file()
{
    this->update();
    return std::filesystem::is_regular_file(this->_fs_path);
}

bool UtilityPath::create_directories()
{
    if (this->_is_single_path_dir)
    {
        // No need to create dirs
        return true;
    }
    return std::filesystem::create_directories(this->_fs_path);
}

int UtilityPath::remove()
{
    this->update();
    return std::filesystem::remove(this->_fs_path);
}

std::ostream &operator<<(std::ostream &output, UtilityPath &report_path)
{
    output << std::string(report_path);
    return output;
}

std::string operator/(std::string &lhs, UtilityPath &rhs)
{
    std::string result = lhs + std::string(rhs);
    return result;
}

std::string operator/(const std::string &lhs, UtilityPath &rhs)
{
    std::string result = lhs + std::string(rhs);
    return result;
}

std::string operator/(std::filesystem::path &lhs, UtilityPath &rhs)
{
    std::string result = std::string(lhs) + std::string(rhs);
    return result;
}

std::uint64_t copyString(char *dst, std::uint64_t size, const std::string &src)
{
    std::uint64_t retval = src.size() + 1;

    if (dst && size > 0)
    {
        std::uint64_t min_size = std::min(size, retval);
        if (min_size > 1)
            std::copy_n(src.c_str(), min_size - 1, dst);
        dst[min_size - 1] = '\0'; // close string
    } // end if

    return retval;
}

void writeToFile(const std::string &filename, std::function<void(std::ostream &)> fn,
                 bool b_binary, bool b_append)
{
    std::fstream output_fnum;

    auto open_flags = (b_append ? std::fstream::app | std::fstream::ate : std::fstream::trunc);
    if (b_binary)
        open_flags |= std::fstream::binary;
    output_fnum.open(filename, std::fstream::out | open_flags);
    if (!output_fnum.is_open())
        throw std::ios_base::failure("Failed to open file \"" + filename + "\" for writing.");
    if (!output_fnum)
        throw std::ios_base::failure("Error after opening file \"" + filename + "\" for writing.");

    fn(output_fnum);

    output_fnum.close();
}

void writeToFile(const std::string &filename,
                 const char *p_data, std::size_t size,
                 bool b_binary, bool b_append)
{
    writeToFile(
        filename,
        [p_data, size](std::ostream &os) -> void {
            os.write(p_data, size);
        },
        b_binary, b_append);
}

void printArraysAsColumns(std::ostream &os,
                          const hebench::APIBridge::NativeDataBuffer **p_buffers, std::size_t count,
                          hebench::APIBridge::DataType data_type,
                          bool output_row_index,
                          const char *separator)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        printArraysAsColumns<std::int32_t>(os, p_buffers, count, output_row_index, separator);
        break;

    case hebench::APIBridge::DataType::Int64:
        printArraysAsColumns<std::int64_t>(os, p_buffers, count, output_row_index, separator);
        break;

    case hebench::APIBridge::DataType::Float32:
        printArraysAsColumns<float>(os, p_buffers, count, output_row_index, separator);
        break;

    case hebench::APIBridge::DataType::Float64:
        printArraysAsColumns<double>(os, p_buffers, count, output_row_index, separator);
        break;

    default:
        printArraysAsColumns<char>(os, p_buffers, count, output_row_index, separator);
        break;
    } // end switch
}

//-----------------------
// class RandomGenerator
//-----------------------

void RandomGenerator::setRandomSeed(std::uint64_t seed)
{
    m_rand.seed(seed);
}

void RandomGenerator::setRandomSeed()
{
    setRandomSeed(std::chrono::system_clock::now().time_since_epoch().count());
}

} // namespace Utilities
} // namespace hebench
