// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _ConfigReader_H_71409a03e9a84aa184b363b098e70fe9
#define _ConfigReader_H_71409a03e9a84aa184b363b098e70fe9

#include <sstream>
#include <string>
#include <unordered_map>

namespace hebench {

class ConfigurationReader
{
public:
    ConfigurationReader();

    void readConfiguration(const std::string &filename, const std::string &magic_value = std::string());
    void readConfiguration(std::istream &is, const std::string &magic_value = std::string());

    const std::string &operator[](const std::string &config) const { return getValue(config); }
    const std::string &getValue(const std::string &config) const;
    bool hasConfig(const std::string &config) const;

    template <class T>
    void getValue(T &value, const std::string &config) const
    {
        if (!hasConfig(config))
            throw std::out_of_range("Invalid configuration key: " + config + ".");

        std::istringstream ss(m_map_config.at(config));
        if (!(ss >> value))
            throw std::logic_error("Invalid value for configuration parameter \"" + config + "\": " + m_map_config.at(config));
    }

    template <class T>
    void getValue(T &value, const std::string &config, const T &default_value) const
    {
        if (!hasConfig(config))
            value = default_value;
        else
            this->getValue<T>(value, config);
    }

    bool empty() const { return m_map_config.empty(); }

private:
    void parseLine(const std::string &s, std::size_t line_no);

    std::unordered_map<std::string, std::string> m_map_config;
};

template <>
inline void ConfigurationReader::getValue<bool>(bool &value, const std::string &config) const
{
    int i_value = 0;
    getValue(i_value, config);
    value = (i_value != 0);
}

} // namespace hebench

#endif
