
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

#include "hebench_report_cpp.h"
#include "hebench_report_overview_header.h"
#include "hebench_report_stats.h"

using namespace std::literals;
using namespace hebench::ReportGen::cpp;

struct ProgramConfig
{
    std::filesystem::path input_file;
    bool b_show_overview;
    bool b_silent;
    char time_unit;
    char time_unit_stats;
    char time_unit_overview;
    char time_unit_summary;

    static constexpr const char *TimeUnit         = "--time_unit";
    static constexpr const char *TimeUnitStats    = "--time_unit_stats";
    static constexpr const char *TimeUnitOverview = "--time_unit_overview";
    static constexpr const char *TimeUnitSummary  = "--time_unit_summary";
    static constexpr const char *ShowOverview     = "--show_overview";
    static constexpr const char *SilentRun        = "--silent_run";

    static constexpr bool DefaultShowOverView = true;

    void initializeConfig(const hebench::ArgsParser &parser);
    void showConfig(std::ostream &os) const;
    static void showVersion(std::ostream &os);
    static std::string getTimeUnitName(char time_unit, const std::string &default_name = "(default)");

private:
    static const std::unordered_set<std::string> m_allowed_time_units;
};

const std::unordered_set<std::string> ProgramConfig::m_allowed_time_units({ "s", "ms", "us", "ns" });

std::string ProgramConfig::getTimeUnitName(char time_unit, const std::string &default_name)
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

void ProgramConfig::initializeConfig(const hebench::ArgsParser &parser)
{
    input_file = parser.getPositionalValue(0);
    input_file = std::filesystem::canonical(input_file);

    parser.getValue<bool>(b_show_overview, ShowOverview, DefaultShowOverView);
    b_silent = parser.hasArgument(SilentRun);

    std::string s_tmp;

    if (parser.hasArgument(TimeUnit))
    {
        parser.getValue<std::string>(s_tmp, TimeUnit);
        if (m_allowed_time_units.count(s_tmp) <= 0)
            throw hebench::ArgsParser::InvalidArgument("Invalid argument value for parameter \"" + std::string(TimeUnit) + "\": " + s_tmp);
    }
    time_unit = s_tmp.empty() ? 0 : s_tmp.front();

    s_tmp.clear();
    if (parser.hasArgument(TimeUnitOverview))
    {
        parser.getValue<std::string>(s_tmp, TimeUnitOverview);
        if (m_allowed_time_units.count(s_tmp) <= 0)
            throw hebench::ArgsParser::InvalidArgument("Invalid argument value for parameter \"" + std::string(TimeUnitOverview) + "\": " + s_tmp);
    }
    time_unit_overview = s_tmp.empty() ? 0 : s_tmp.front();

    s_tmp.clear();
    if (parser.hasArgument(TimeUnitStats))
    {
        parser.getValue<std::string>(s_tmp, TimeUnitStats);
        if (m_allowed_time_units.count(s_tmp) <= 0)
            throw hebench::ArgsParser::InvalidArgument("Invalid argument value for parameter \"" + std::string(TimeUnitStats) + "\": " + s_tmp);
    }
    time_unit_stats = s_tmp.empty() ? 0 : s_tmp.front();

    s_tmp.clear();
    if (parser.hasArgument(TimeUnitSummary))
    {
        parser.getValue<std::string>(s_tmp, TimeUnitSummary);
        if (m_allowed_time_units.count(s_tmp) <= 0)
            throw hebench::ArgsParser::InvalidArgument("Invalid argument value for parameter \"" + std::string(TimeUnitSummary) + "\": " + s_tmp);
    }
    time_unit_summary = s_tmp.empty() ? 0 : s_tmp.front();
}

void ProgramConfig::showConfig(std::ostream &os) const
{
    std::string fallback_time_unit = getTimeUnitName(time_unit);
    os << "==================" << std::endl
       << "Global Configuration:" << std::endl
       << "    Input file: " << input_file << std::endl
       << "    Show overview: " << (b_show_overview ? "Yes" : "No") << std::endl
       << "    Overview time unit: " << (time_unit_overview == 0 ? fallback_time_unit : getTimeUnitName(time_unit_overview)) << std::endl
       << "    Summary time unit: " << (time_unit_summary == 0 ? fallback_time_unit : getTimeUnitName(time_unit_summary)) << std::endl
       << "    Statistics time unit: " << (time_unit_stats == 0 ? fallback_time_unit : getTimeUnitName(time_unit_stats)) << std::endl;
    os << "==================" << std::endl;
}

void ProgramConfig::showVersion(std::ostream &os)
{
    (void)os;
}

void initArgsParser(hebench::ArgsParser &parser, int argc, char **argv)
{
    parser.addPositionalArgument("input_file",
                                 "    Input file to be used during report generation. This can be a single\n"
                                 "    benchmark report generated by Test Harness or a file containing a list\n"
                                 "    of report files to process. The directory for this file must be\n"
                                 "    available for writing.");
    parser.addArgument(ProgramConfig::TimeUnit, "-tu", 1, "<time_unit_name>",
                       "    [OPTIONAL] Specifies the time unit to be used for the generated reports\n"
                       "    if no time unit is specified for a report type. The value of this\n"
                       "    parameter must be one of:\n"
                       "      \"s\"  - seconds\n"
                       "      \"ms\" - milliseconds\n"
                       "      \"us\" - microseconds\n"
                       "      \"ns\" - nanoseconds\n"
                       "    If missing, an appropriapriate unit will be automatically selected per\n"
                       "    benchmark.");
    parser.addArgument(ProgramConfig::TimeUnitOverview, "-tuo", 1, "<time_unit_name>",
                       "    [OPTIONAL] Specifies the time unit to be used for the generated overview.\n"
                       "    If no time unit is specified, the fallback time unit specified for\n"
                       "    parameter `"
                           + std::string(ProgramConfig::TimeUnit) + "` is used instead.");
    parser.addArgument(ProgramConfig::TimeUnitStats, "-tut", 1, "<time_unit_name>",
                       "    [OPTIONAL] Specifies the time unit to be used for the generated\n"
                       "    statistics. If no time unit is specified, the fallback time unit\n"
                       "    specified for parameter `"
                           + std::string(ProgramConfig::TimeUnit) + "` is used instead.");
    parser.addArgument(ProgramConfig::TimeUnitSummary, "-tus", 1, "<time_unit_name>",
                       "    [OPTIONAL] Specifies the time unit to be used for the generated\n"
                       "    summaries. If no time unit is specified, the fallback time unit\n"
                       "    specified for parameter `"
                           + std::string(ProgramConfig::TimeUnit) + "` is used instead.");
    parser.addArgument(ProgramConfig::ShowOverview, "--overview", 1, "<true | false | 1 | 0>",
                       "    [OPTIONAL] Specifies whether or not to display report overview to\n"
                       "    standard output. If option is missing, default is \"true\".");
    parser.addArgument(ProgramConfig::SilentRun, "--silent", 0, "",
                       "    [OPTIONAL] When present, this flag to indicates that the run must be\n"
                       "    silent. Only specifically requested outputs will be displayed to\n"
                       "    standard output.");
    parser.parse(argc, argv);
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

    bool b_done = false;
    std::string s_line;
    std::size_t line_num = 0;
    while (!b_done && std::getline(fnum, s_line))
    {
        ++line_num;
        if (!s_line.empty())
        {
            if (std::filesystem::is_regular_file(s_line))
            {
                retval.emplace_back(s_line);
            } // end if
            else // line is not a file
            {
                if (!retval.empty())
                {
                    // other lines were already added as files, so,
                    // error out because this line points to non-existing file
                    std::stringstream ss;
                    ss << filename << ":" << line_num << ": file specified in line not found.";
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

int main(int argc, char **argv)
{
    static const std::string ReportSuffix = "report";

    int retval = 0;

    ProgramConfig config;
    hebench::ArgsParser args_parser;

    try
    {
        std::cout << "HEBench Report Compiler" << std::endl
                  << std::endl;

        initArgsParser(args_parser, argc, argv);
        config.initializeConfig(args_parser);

        if (!config.b_silent)
        {
            config.showConfig(std::cout);
            std::cout << std::endl;
        } // end if

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

        ss_overview_header << ",,,,,,,,,,,,Wall Time,,,,,,,,,,,,,CPU Time" << std::endl
                           << "Workload,Filename,Category,Data type,Cipher text,Scheme,Security,"
                           << "ID,Event,Total Wall Time,Ops per sec,Ops per sec trimmed,"
                           // wall
                           << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,"
                           // cpu
                           << "Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Iterations";

        for (std::size_t csv_file_i = 0; csv_file_i < csv_filenames.size(); ++csv_file_i)
        {
            if (!config.b_silent)
            {
                std::cout << "=====================" << std::endl
                          << " Progress: " << csv_file_i << "/" << csv_filenames.size() << std::endl
                          << "           " << hebench::Utilities::convertDoubleToStr(csv_file_i * 100.0 / csv_filenames.size(), 2) << "%" << std::endl
                          << "=====================" << std::endl
                          << std::endl;

                std::cout << "Report file: " << csv_filenames[csv_file_i] << std::endl;
            } // end if

            if (!config.b_silent)
            {
                std::cout << "Loading report..." << std::endl;
            } // end if

            TimingReport report =
                TimingReport::loadReportFromCSVFile(csv_filenames[csv_file_i]);

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

            if (report.getEventCount() <= 0)
            {
                if (!config.b_silent)
                {
                    std::cout << std::endl
                              << "The loaded report belongs to a failed benchmark." << std::endl;
                } // end if

                ss_overview << "Failed";
            } // end if
            else
            {
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
                // add the workload parameters
                for (std::size_t i = 0; i < overview_header.w_params.size(); ++i)
                {
                    if (overview_header.w_params[i].find_first_of(',') == std::string::npos)
                        ss_overview << ",\"" << overview_header.w_params[i] << "\"";
                    else
                        ss_overview << "," << overview_header.w_params[i];
                } // end for
            } // end else

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
    }
    catch (hebench::ArgsParser::HelpShown &)
    {
        // no error
    }
    catch (hebench::ArgsParser::InvalidArgument &ap_ex)
    {
        std::cout << "Error occurred with message: " << std::endl
                  << ap_ex.what() << std::endl
                  << std::endl;
        args_parser.printUsage();
        retval = -1;
    }
    catch (std::exception &ex)
    {
        std::cout << "Error occurred with message: " << std::endl
                  << ex.what() << std::endl;
        retval = -1;
    }
    catch (...)
    {
        std::cout << "Unexpected error occurred!" << std::endl;
        retval = -1;
    }

    if (retval)
        std::cout << std::endl
                  << "Terminated with errors." << std::endl;
    else
        std::cout << std::endl
                  << "Complete!" << std::endl;

    return retval;
}
