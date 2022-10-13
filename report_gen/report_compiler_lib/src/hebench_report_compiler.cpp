
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "modules/args_parser/include/args_parser.h"
#include "modules/general/include/hebench_utilities.h"

#include "hebench_report_compiler.h"
#include "hebench_report_cpp.h"
#include "hebench_report_overview_header.h"
#include "hebench_report_stats.h"

namespace hebench {
namespace ReportGen {
namespace Compiler {

using namespace std::literals;
using namespace hebench::ReportGen::cpp;

struct ReportCompilerConfig
{
    std::filesystem::path input_file;
    bool b_show_overview;
    bool b_silent;
    char time_unit;
    char time_unit_stats;
    char time_unit_overview;
    char time_unit_summary;

    void showConfig(std::ostream &os) const;
    static void showVersion(std::ostream &os);
    static std::string getTimeUnitName(char time_unit, const std::string &default_name = "(default)");

private:
    static const std::unordered_set<std::string> m_allowed_time_units;
};

const std::unordered_set<std::string> ReportCompilerConfig::m_allowed_time_units({ "s", "ms", "us", "ns" });

std::string ReportCompilerConfig::getTimeUnitName(char time_unit, const std::string &default_name)
{
    std::string retval;
    switch (time_unit)
    {
    case 'm':
        retval = "milliseconds";
        break;
    case 'u':
        retval = "microseconds";
        break;
    case 'n':
        retval = "nanoseconds";
        break;
    default:
        retval = default_name;
        break;
    } // end switch
    return retval;
}

void ReportCompilerConfig::showVersion(std::ostream &os)
{
    (void)os;
}

std::vector<std::filesystem::path> extractInputFiles(const std::string &filename)
{
    std::vector<std::filesystem::path> retval;

    // Open file and see if each line contains a valid, existing filename.
    // Throw exception if a file does not exist after the first one.

    std::ifstream fnum;
    fnum.open(filename, std::ios_base::in);
    if (!fnum.is_open())
        throw std::ios_base::failure("Could not open file \"" + filename + "\"");

    std::filesystem::path root_path = std::filesystem::canonical(filename).parent_path();

    bool b_done = false;
    std::string s_line;
    std::size_t line_num = 0;
    while (!b_done && std::getline(fnum, s_line))
    {
        ++line_num;
        hebench::Utilities::trim(s_line);
        if (!s_line.empty())
        {
            std::filesystem::path next_filename = s_line;
            if (next_filename.is_relative())
                // relative paths are relative to input file
                next_filename = root_path / next_filename;

            if (std::filesystem::is_regular_file(next_filename))
            {
                retval.emplace_back(next_filename.string());
            } // end if
            else // line is not a file
            {
                if (!retval.empty())
                {
                    // other lines were already added as files, so,
                    // error out because this line points to non-existing file
                    std::stringstream ss;
                    ss << filename << ":" << line_num << ": file specified in line not found: " << next_filename;
                    throw std::invalid_argument(ss.str());
                } // end if

                // first non-empty line is not an existing file, so,
                // this filename itself could be the file to read
                b_done = true;
            } // end if
        } // end if
    } // end while

    if (retval.empty())
        retval.emplace_back(filename);

    return retval;
}

extern "C"
{

    int32_t compile(const ReportCompilerConfigC *p_config, char *s_error, size_t s_error_size)
    {
        static const std::string ReportSuffix = "report";

        int retval = 1;

        std::stringstream ss_err;
        ReportCompilerConfig config;

        try
        {
            if (!p_config || !p_config->input_file)
                throw std::runtime_error("Invalid null compiler configuration values.");

            config.input_file         = p_config->input_file;
            config.b_show_overview    = p_config->b_show_overview;
            config.b_silent           = p_config->b_silent;
            config.time_unit          = p_config->time_unit;
            config.time_unit_stats    = p_config->time_unit_stats;
            config.time_unit_overview = p_config->time_unit_overview;
            config.time_unit_summary  = p_config->time_unit_summary;

            if (!config.b_silent)
            {
                std::cout << "Extracting input file names..." << std::endl
                          << std::endl;
            } // end if

            std::size_t max_w_params = 0;
            std::stringstream ss_overview_header;
            std::stringstream ss_overview;
            std::filesystem::path overview_filename = config.input_file;
            overview_filename.replace_filename(overview_filename.stem().string() + "_overview.csv");
            std::vector<std::filesystem::path> csv_filenames = extractInputFiles(config.input_file);

            if (!config.b_silent)
            {
                std::cout << "Input files:" << std::endl;
                for (std::size_t i = 0; i < csv_filenames.size(); ++i)
                    std::cout << "  " << i << ", " << csv_filenames[i] << std::endl;
                std::cout << std::endl
                          << "Overview file (output):" << std::endl
                          << "  " << overview_filename << std::endl
                          << std::endl;
            } // end if

            ss_overview_header << ",,,,,,,,,,,,,Wall Time,,,,,,,,,,,,,CPU Time" << std::endl
                               << "Workload,Filename,Category,Data type,Cipher text,Scheme,Security,Extra,"
                               << "ID,Event,Total Wall Time,Samples per sec,Samples per sec trimmed,"
                               // wall
                               << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,"
                               // cpu
                               << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Input Samples";

            for (std::size_t csv_file_i = 0; csv_file_i < csv_filenames.size(); ++csv_file_i)
            {
                if (!config.b_silent)
                {
                    std::cout << "=====================" << std::endl
                              << " Progress: " << csv_file_i << "/" << csv_filenames.size() << std::endl
                              << "           " << hebench::Utilities::convertDoubleToStr(csv_file_i * 100.0 / csv_filenames.size(), 2) << "%" << std::endl
                              << "=====================" << std::endl
                              << std::endl;

                    std::cout << "Report file:" << std::endl
                              << "  ";
                } // end if
                std::cerr << csv_filenames[csv_file_i] << std::endl;

                if (!config.b_silent)
                {
                    std::cout << "Loading report..." << std::endl;
                } // end if

                std::shared_ptr<TimingReport> p_report;

                try
                {
                    p_report = std::make_shared<TimingReport>(TimingReport::loadReportFromCSVFile(csv_filenames[csv_file_i]));
                }
                catch (...)
                {
                }

                if (p_report)
                {
                    TimingReport &report = *p_report;

                    if (report.getEventCount() <= 0)
                    {
                        std::cerr << "WARNING: The loaded report belongs to a failed benchmark." << std::endl;
                        ss_overview << "Failed," << csv_filenames[csv_file_i] << ",Validation";
                    } // end if
                    else
                    {
                        if (!config.b_silent)
                        {
                            std::cout << "Parsing report header..." << std::endl;
                        } // end if

                        hebench::ReportGen::OverviewHeader overview_header;
                        overview_header.parseHeader(csv_filenames[csv_file_i], report.getHeader());
                        // make sure we keep track of the workload parameters
                        if (overview_header.w_params.size() > max_w_params)
                        {
                            for (std::size_t i = max_w_params; i < overview_header.w_params.size(); ++i)
                                ss_overview_header << ",wp" << i;
                            max_w_params = overview_header.w_params.size();
                        } // end if
                        overview_header.outputHeader(ss_overview, false);
                        ss_overview << ",";

                        char tmp_time_unit;
                        std::filesystem::path stem_filename = csv_filenames[csv_file_i];
                        std::filesystem::path summary_filename;
                        std::filesystem::path stats_filename;

                        // remove "report" from input file name
                        std::string s_tmp = stem_filename.stem();
                        if (s_tmp.length() >= ReportSuffix.length() && s_tmp.substr(s_tmp.length() - ReportSuffix.length()) == ReportSuffix)
                            stem_filename.replace_filename(s_tmp.substr(0, s_tmp.length() - ReportSuffix.length()));
                        summary_filename = stem_filename;
                        summary_filename += "summary.csv";
                        stats_filename = stem_filename;
                        stats_filename += "stats.csv";

                        if (!config.b_silent)
                        {
                            std::cout << "Computing statistics..." << std::endl;
                        } // end if

                        hebench::ReportGen::ReportStats report_stats(report);

                        if (!config.b_silent)
                        {
                            std::cout << "Writing summary to:" << std::endl
                                      << "  " << summary_filename << std::endl;
                        } // end if

                        tmp_time_unit = config.time_unit_summary;
                        hebench::Utilities::writeToFile(
                            summary_filename,
                            [&report_stats, tmp_time_unit](std::ostream &os) -> void {
                                report_stats.generateSummaryCSV(os, tmp_time_unit);
                            },
                            false);

                        if (!config.b_silent)
                        {
                            std::cout << "Writing statistics to:" << std::endl
                                      << "  " << stats_filename << std::endl;
                        } // end if

                        tmp_time_unit = config.time_unit_stats;
                        hebench::Utilities::writeToFile(
                            stats_filename,
                            [&report_stats, tmp_time_unit](std::ostream &os) -> void {
                                report_stats.generateCSV(os, tmp_time_unit);
                            },
                            false);

                        if (!config.b_silent)
                        {
                            std::cout << "Adding to report overview..." << std::endl;
                        } // end if

                        report_stats.generateCSV(ss_overview, report_stats.getMainEventTypeStats(), config.time_unit_overview, false);
                        // add the workload parameters if any was found
                        if (overview_header.w_params.empty())
                        {
                            // on file name
                            std::cerr << "WARNING: No workload parameters found while parsing." << std::endl;
                        } // end if
                        for (std::size_t i = 0; i < overview_header.w_params.size(); ++i)
                        {
                            if (overview_header.w_params[i].find_first_of(',') == std::string::npos)
                                ss_overview << "," << overview_header.w_params[i];
                            else
                                ss_overview << ",\"" << overview_header.w_params[i] << "\"";
                        } // end for
                    } // end else
                } // end if
                else
                {
                    std::cerr << "WARNING: Failed to load report from file." << std::endl;
                    ss_overview << "Failed," << csv_filenames[csv_file_i] << ",Load";
                } // end else

                if (!config.b_silent)
                    std::cout << std::endl;

                ss_overview << std::endl;
            } // end for

            ss_overview_header << std::endl
                               << ss_overview.str();
            ss_overview            = std::stringstream();
            std::string s_overview = ss_overview_header.str();
            ss_overview_header     = std::stringstream();
            hebench::Utilities::writeToFile(overview_filename, s_overview.c_str(), s_overview.size() * sizeof(char), false);

            if (!config.b_silent)
            {
                std::cout << "=====================" << std::endl
                          << " Progress: " << csv_filenames.size() << "/" << csv_filenames.size() << std::endl
                          << "           100%" << std::endl
                          << "=====================" << std::endl
                          << std::endl;
            } // end if

            if (config.b_show_overview)
            {
                if (!config.b_silent)
                    std::cout << "Overview:" << std::endl;
                std::cout << std::endl
                          << s_overview << std::endl;
            } // end if
        }
        catch (std::exception &ex)
        {
            ss_err << ex.what();
            retval = 0;
        }
        catch (...)
        {
            ss_err << "Unexpected error occurred.";
            retval = 0;
        }

        if (!retval)
            // report error message
            hebench::Utilities::copyString(s_error, s_error_size, ss_err.str());

        return retval;
    }
}

} // namespace Compiler
} // namespace ReportGen
} // namespace hebench
