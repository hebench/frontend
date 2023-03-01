
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "hebench/modules/args_parser/include/args_parser.h"

#include "hebench_report_compiler.h"

using namespace std::literals;

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
                       "    If missing, an appropriate unit will be automatically selected per\n"
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
                       "    [OPTIONAL] When present, this flag indicates that the run must be\n"
                       "    silent. Only specifically requested outputs will be displayed to\n"
                       "    standard output. When running silent, important messages, warnings and\n"
                       "    errors will be sent to standard error stream `stderr`.");
    parser.parse(argc, argv);
}

int main(int argc, char **argv)
{
    int retval = 0;

    std::stringstream ss_err;
    ProgramConfig config;
    hebench::ArgsParser args_parser;

    try
    {
        initArgsParser(args_parser, argc, argv);
        config.initializeConfig(args_parser);

        if (!config.b_silent)
        {
            std::cout << "HEBench Report Compiler" << std::endl
                      << std::endl;
            config.showConfig(std::cout);
            std::cout << std::endl;
        } // end if

        hebench::ReportGen::Compiler::ReportCompilerConfigC compile_config;
        std::string s_input_file = config.input_file.string();
        std::vector<char> c_error_msg(1024, 0);

        compile_config.input_file         = s_input_file.c_str();
        compile_config.b_show_overview    = config.b_show_overview ? 1 : 0;
        compile_config.b_silent           = config.b_silent ? 1 : 0;
        compile_config.time_unit          = config.time_unit;
        compile_config.time_unit_stats    = config.time_unit_stats;
        compile_config.time_unit_overview = config.time_unit_overview;
        compile_config.time_unit_summary  = config.time_unit_summary;

        if (!hebench::ReportGen::Compiler::compile(&compile_config, c_error_msg.data(), c_error_msg.size()))
            throw std::runtime_error(c_error_msg.data());
    }
    catch (hebench::ArgsParser::HelpShown &)
    {
        // no error
    }
    catch (hebench::ArgsParser::InvalidArgument &ap_ex)
    {
        ss_err << "Error occurred with message: " << std::endl
               << ap_ex.what() << std::endl
               << std::endl;
        args_parser.printUsage(ss_err);
        retval = -1;
    }
    catch (std::exception &ex)
    {
        ss_err << "ERROR: " << std::endl
               << ex.what() << std::endl;
        retval = -1;
    }
    catch (...)
    {
        ss_err << "Unexpected error occurred!" << std::endl;
        retval = -1;
    }

    if (retval)
    {
        std::cerr << std::endl
                  << ss_err.str() << std::endl
                  << "Terminated with errors." << std::endl;
    } // end if
    else if (!config.b_silent)
        std::cout << std::endl
                  << "Complete!" << std::endl;

    return retval;
}
