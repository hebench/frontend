
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <sstream>
#include <utility>

#include "modules/general/include/hebench_utilities.h"

#include "hebench_report_overview_header.h"

using namespace std::literals;

namespace hebench {
namespace ReportGen {

std::vector<std::string_view> OverviewHeader::extractInfoFromCSVLine(std::string_view s_row, const std::string_view &s_tag, std::size_t num_values)
{
    std::vector<std::string_view> retval;

    retval.reserve(num_values);
    hebench::Utilities::trim(s_row);
    auto pos = s_row.find(s_tag);
    if (pos != std::string_view::npos)
    {
        s_row.remove_prefix(pos + s_tag.size());
        while (!s_row.empty() && retval.size() < num_values)
        {
            hebench::Utilities::ltrim(s_row);
            char ch_to_find = ',';
            if (!s_row.empty() && s_row.front() == '"')
            {
                // deal with quotations
                ch_to_find = '"';
                s_row.remove_prefix(1);
            }
            pos = s_row.find_first_of(ch_to_find);
            if (pos == std::string_view::npos)
            {
                pos = s_row.size();
            }
            retval.emplace_back(s_row.substr(0, pos));
            hebench::Utilities::trim(retval.back());
            s_row.remove_prefix(pos);

            if (!s_row.empty())
            {
                // remove character delimiter found
                s_row.remove_prefix(1);
                if (ch_to_find == '"')
                {
                    // ignore the rest of the value until next delimiter
                    pos = s_row.find_first_of(',');
                    if (pos == std::string_view::npos)
                        s_row = std::string_view();
                    else
                        s_row.remove_prefix(pos + 1);
                } // end if
            } // end if
        } // end while
    } // end if

    return retval;
}

void OverviewHeader::parseHeader(const std::string &filename, const std::string &s_header)
{
    static const std::string s_trim = std::string(",") + hebench::Utilities::BlankTrim;

    std::stringstream ss(s_header);
    std::string s_line;
    while (std::getline(ss, s_line))
    {
        hebench::Utilities::trim(s_line, s_trim.c_str());
        std::vector<std::string_view> values;

        if (this->workload_name.empty())
        {
            values = extractInfoFromCSVLine(s_line, "Workload,", 1);
            if (values.size() > 0)
                this->workload_name.assign(values.front().begin(), values.front().end());
        } // end if
        if (this->category.empty())
        {
            values = extractInfoFromCSVLine(s_line, "Category,", 1);
            if (values.size() > 0)
                this->category.assign(values.front().begin(), values.front().end());
        } // end if
        if (this->data_type.empty())
        {
            values = extractInfoFromCSVLine(s_line, "Data type,", 1);
            if (values.size() > 0)
                this->data_type.assign(values.front().begin(), values.front().end());
        } // end if
        if (this->scheme.empty())
        {
            values = extractInfoFromCSVLine(s_line, "Scheme,", 1);
            if (values.size() > 0)
                this->scheme.assign(values.front().begin(), values.front().end());
        } // end if
        if (this->security.empty())
        {
            values = extractInfoFromCSVLine(s_line, "Scheme,", 1);
            if (values.size() > 0)
                this->security.assign(values.front().begin(), values.front().end());
        } // end if
    } // end while

    this->report_file = filename;
    // extract workload parameters from file name
    std::string_view s_view = this->report_file;
    auto pos                = s_view.find("wp_");
    if (pos != std::string_view::npos)
    {
        s_view.remove_prefix(pos + "wp_"sv.size());
        pos = s_view.find_first_of("-/\\");
        if (pos != std::string_view::npos)
            s_view.remove_suffix(s_view.size() - pos);
        // s_view now has only the workload params separated by underscores
        auto tokens = hebench::Utilities::tokenize(s_view, "_");
        for (std::size_t i = 0; i < tokens.size(); ++i)
            this->w_params.emplace_back(tokens[i].begin(), tokens[i].end());
    } // end if
}

void OverviewHeader::outputHeader(std::ostream &os, bool new_line)
{
    (this->workload_name.find_first_of(',') == std::string::npos ?
         os << this->workload_name :
         os << "\"" << this->workload_name << "\"");
    os << ",";
    (this->report_file.find_first_of(',') == std::string::npos ?
         os << this->report_file :
         os << "\"" << this->report_file << "\"");
    os << ",";
    (this->category.find_first_of(',') == std::string::npos ?
         os << this->category :
         os << "\"" << this->category << "\"");
    os << ",";
    (this->data_type.find_first_of(',') == std::string::npos ?
         os << this->data_type :
         os << "\"" << this->data_type << "\"");
    os << ",";
    (this->cipher_text.find_first_of(',') == std::string::npos ?
         os << this->cipher_text :
         os << "\"" << this->cipher_text << "\"");
    os << ",";
    (this->scheme.find_first_of(',') == std::string::npos ?
         os << this->scheme :
         os << "\"" << this->scheme << "\"");
    os << ",";
    (this->security.find_first_of(',') == std::string::npos ?
         os << this->security :
         os << "\"" << this->security << "\"");
    if (new_line)
        os << std::endl;
}

} // namespace ReportGen
} // namespace hebench
