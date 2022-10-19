
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace hebench {
namespace ReportGen {

struct OverviewHeader
{
public:
    static constexpr const char *EndStateOK             = "OK";
    static constexpr const char *EndStateGeneralFailure = "Failed";

    OverviewHeader() :
        other(0) {}

    std::string workload_name;
    std::string end_state;
    std::string report_file;
    std::string category;
    std::string data_type;
    std::string cipher_text;
    std::string scheme;
    std::string security;
    std::int64_t other;
    std::vector<std::string> w_params;

    void parseHeader(const std::string &filename, const std::string &s_header, const std::string &s_end_state);

    // outputs header without workload parameters
    void outputHeader(std::ostream &os, bool new_line = true);

private:
    static std::vector<std::string_view> extractInfoFromCSVLine(std::string_view s_row,
                                                                const std::string_view &s_tag,
                                                                std::size_t num_values);
    static std::string extractCiphertextBitset(std::vector<std::string_view> s_indices);
};

} // namespace ReportGen
} // namespace hebench

#endif // defined _HEBench_ReportHeader_H_0596d40a3cce4b108a81595c50eb286d
