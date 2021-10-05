// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "../include/config_reader.h"

#include <fstream>
#include <stdexcept>

using namespace hebench;

ConfigurationReader::ConfigurationReader() {}

void ConfigurationReader::readConfiguration(const std::string &filename, const std::string &magic_value)
{
    std::ifstream fnum_input;

    try
    {
        fnum_input.open(filename, std::ifstream::in);
        if (!fnum_input.is_open())
            throw std::ios_base::failure("Cannot open file " + filename);

        readConfiguration(fnum_input, magic_value);
    }
    catch (...)
    {
        if (fnum_input.is_open())
            fnum_input.close();

        throw;
    }

    if (fnum_input.is_open())
        fnum_input.close();
}

void ConfigurationReader::readConfiguration(std::istream &is, const std::string &magic_value)
{
    // read the stream until the end
    std::string sline;
    std::size_t line_cnt = 0;
    while (std::getline(is, sline))
    {
        line_cnt++; // new line read

        // check for magic value
        if (line_cnt == 1 && !magic_value.empty())
        {
            if (magic_value != sline)
                throw std::logic_error("Invalid format detected in line " + std::to_string(line_cnt) + ": invalid heading detected.");
        }
        else
            parseLine(sline, line_cnt);
    }
}

void ConfigurationReader::parseLine(const std::string &s, std::size_t line_no)
{
    std::string local = s;

    // remove leading and trailing blanks
    local.erase(0, local.find_first_not_of(" \t\n\r\f\v"));
    local.erase(local.find_last_not_of(" \t\n\r\f\v") + 1);

    // remove any comments
    std::size_t comment_pos = local.find_first_of("#");
    if (comment_pos != std::string::npos)
        local.erase(comment_pos);

    if (!local.empty())
    {
        // find equal sign
        std::size_t equal_pos = local.find_last_of('=');
        if (equal_pos == std::string::npos)
            throw std::logic_error("Invalid format detected in line " + std::to_string(line_no) + ": no equal sign found.");

        // retrieve config key
        std::string key = local.substr(0, equal_pos);
        // remove trailing blanks
        key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);

        if (key.empty())
            throw std::logic_error("Invalid format detected in line " + std::to_string(line_no) + ": no key found.");
        if (m_map_config.count(key) > 0)
            throw std::logic_error("Invalid format detected in line " + std::to_string(line_no) + ": duplicated key \"" + key + "\".");

        // retrieve config value
        std::string value = local.substr(equal_pos + 1);
        // remove leading blanks
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));

        // add config pair to map
        // m_map_config.insert(std::make_pair<std::string, std::string>(key,
        // value));
        m_map_config[key] = value;
    }
}

const std::string &ConfigurationReader::getValue(const std::string &config) const
{
    if (hasConfig(config))
        return const_cast<ConfigurationReader *>(this)->m_map_config[config];

    throw std::out_of_range("Invalid configuration key: " + config + ".");
}

bool ConfigurationReader::hasConfig(const std::string &config) const
{
    return m_map_config.count(config) > 0;
}
