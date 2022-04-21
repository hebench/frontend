
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace hebench {
namespace ReportGen {

struct OverviewHeader
{
public:
    std::string workload_name;
    std::string report_file;
    std::string category;
    std::string data_type;
    std::string cipher_text;
    std::string scheme;
    std::string security;
    std::vector<std::string> w_params;

    void parseHeader(const std::string &filename, const std::string &s_header);

    // outputs header without workload parameters
    void outputHeader(std::ostream &os, bool new_line = true);

private:
    static std::vector<std::string_view> extractInfoFromCSVLine(std::string_view s_row,
                                                                const std::string_view &s_tag,
                                                                std::size_t num_values);
};

} // namespace ReportGen
} // namespace hebench

#endif // defined _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d
