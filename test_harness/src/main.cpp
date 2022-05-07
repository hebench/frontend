
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "modules/args_parser/include/args_parser.h"
#include "modules/general/include/error.h"
#include "modules/general/include/hebench_math_utils.h"
#include "modules/general/include/hebench_utilities.h"
#include "modules/logging/include/logging.h"

#include "dynamic_lib_load.h"
#include "hebench_report_compiler.h"

#include "include/hebench_config.h"
#include "include/hebench_engine.h"
#include "include/hebench_types_harness.h"
#include "include/hebench_utilities_harness.h"
#include "include/hebench_version.h"

// enforce floating point standard compatibility
static_assert(std::numeric_limits<float>::is_iec559, "Compiler type `float` does not comply with IEEE 754.");
static_assert(std::numeric_limits<double>::is_iec559, "Compiler type `double` does not comply with IEEE 754.");
static_assert(sizeof(float) == 4, "Compiler type `float` is not 32 bits.");
static_assert(sizeof(double) == 8, "Compiler type `double` is not 64 bits.");

struct ProgramConfig
{
    std::filesystem::path backend_lib_path;
    std::filesystem::path config_file;
    bool b_dump_config;
    bool b_validate_results;
    bool b_single_path_report;
    std::uint64_t random_seed;
    std::size_t report_delay_ms;
    std::filesystem::path report_root_path;
    bool b_show_run_overview;
    bool b_compile_reports;

    static constexpr const char *DefaultConfigFile    = "";
    static constexpr std::uint64_t DefaultMinTestTime = 0;
    static constexpr std::uint64_t DefaultSampleSize  = 0;
    static constexpr std::size_t DefaultReportDelay   = 1000;
    static constexpr const char *DefaultRootPath      = ".";

    void initializeConfig(const hebench::ArgsParser &parser);
    void showBenchmarkDefaults(std::ostream &os);
    void showConfig(std::ostream &os) const;
    static std::ostream &showVersion(std::ostream &os);
};

void ProgramConfig::initializeConfig(const hebench::ArgsParser &parser)
{
    std::string s_tmp;

    if (parser.hasArgument("--version"))
    {
        showVersion(std::cout);
        throw hebench::ArgsParser::HelpShown("Version shown.");
    } // end if

    parser.getValue<decltype(s_tmp)>(s_tmp, "--benchmark_config_file", DefaultConfigFile);
    config_file = s_tmp;

    b_dump_config = parser.hasArgument("--dump_config");
    if (b_dump_config && config_file.empty())
        throw std::runtime_error("Dump default benchmark configuration file requested, but no filename given with \"--benchmark_config_file\" parameter.");

    parser.getValue<decltype(s_tmp)>(s_tmp, "--backend_lib_path");
    backend_lib_path = s_tmp;

    parser.getValue<decltype(b_validate_results)>(b_validate_results, "--enable_validation", true);

    parser.getValue<decltype(random_seed)>(random_seed, "--random_seed", std::chrono::system_clock::now().time_since_epoch().count());

    parser.getValue<decltype(report_delay_ms)>(report_delay_ms, "--report_delay", DefaultReportDelay);

    parser.getValue<decltype(s_tmp)>(s_tmp, "--report_root_path", DefaultRootPath);
    report_root_path = s_tmp;
    if (!std::filesystem::is_directory(report_root_path) || !std::filesystem::exists(report_root_path))
        throw std::runtime_error("Specified directory for report output does not exists or is not accessible: " + report_root_path.string());
    if (!std::filesystem::is_regular_file(backend_lib_path) || !std::filesystem::exists(backend_lib_path))
        throw std::runtime_error("Specified backend lib does not exists or is not accessible: " + backend_lib_path.string());
    if (std::filesystem::is_symlink(backend_lib_path))
        throw std::runtime_error("Backend library error: symbolic links are not allowed as input arguments: " + backend_lib_path.string());
    if ((std::filesystem::canonical(backend_lib_path).string()).substr(0, 5) == std::string("/tmp/"))
        throw std::runtime_error("Backend library error: Cannot use files in /tmp/ as arguments: " + backend_lib_path.string());

    if (!b_dump_config && !config_file.empty())
    {
        // reading configuration file
        if (!std::filesystem::is_regular_file(config_file) || !std::filesystem::exists(config_file))
            throw std::runtime_error("Specified benchmark configuration file does not exists or is not accessible: " + config_file.string());
        if (std::filesystem::is_symlink(config_file))
            throw std::runtime_error("Config file path error: symbolic links are not allowed as input arguments: " + config_file.string());
        if (std::filesystem::canonical(config_file).string().substr(0, 5) == std::string("/tmp/"))
            throw std::runtime_error("Config file error: Cannot use files in /tmp/ as arguments: " + config_file.string());
    }

    parser.getValue<decltype(b_show_run_overview)>(b_show_run_overview, "--run_overview", true);
    parser.getValue<decltype(b_compile_reports)>(b_compile_reports, "--compile_reports", true);

    b_single_path_report = parser.hasArgument("--single_path_report");
}

void ProgramConfig::showBenchmarkDefaults(std::ostream &os)
{
    os << "Benchmark defaults:" << std::endl
       << "    Random seed: " << random_seed << std::endl;
}

void ProgramConfig::showConfig(std::ostream &os) const
{
    os << "Global Configuration:" << std::endl
       << "    Backend library: " << backend_lib_path << std::endl
       //<< std::endl
       << "    ==================" << std::endl
       << "    Run type: ";
    if (b_dump_config)
    {
        os << "Dumping configuration file!" << std::endl;
    } // end of
    else
    {
        os << "Benchmark Run." << std::endl
           << "    Validate results: " << (b_validate_results ? "Yes" : "No") << std::endl
           << "    Report delay (ms): " << report_delay_ms << std::endl
           << "    Report Root Path: " << report_root_path << std::endl
           << "    Compile reports: " << (b_compile_reports ? "Yes" : "No") << std::endl
           << "    Show run overview: " << (b_show_run_overview ? "Yes" : "No") << std::endl;
    } // end if
    os << "    Run configuration file: ";
    if (config_file.empty())
        os << "(none)" << std::endl;
    else
        os << config_file << std::endl;
    os << "    ==================" << std::endl;
}

std::ostream &ProgramConfig::showVersion(std::ostream &os)
{
    os << HEBENCH_TEST_HARNESS_APP_NAME << " v"
       << HEBENCH_TEST_HARNESS_VERSION_MAJOR << "."
       << HEBENCH_TEST_HARNESS_VERSION_MINOR << "."
       << HEBENCH_TEST_HARNESS_VERSION_REVISION << "-"
       << HEBENCH_TEST_HARNESS_VERSION_BUILD << std::endl
       << std::endl
       << "API Bridge version:" << std::endl
       << "  Required: " << HEBENCH_TEST_HARNESS_API_REQUIRED_VERSION_MAJOR << "."
       << HEBENCH_TEST_HARNESS_API_REQUIRED_VERSION_MINOR << "."
       << HEBENCH_TEST_HARNESS_API_MIN_REQUIRED_VERSION_REVISION << std::endl
       << "  Current:  "
       << HEBENCH_API_VERSION_MAJOR << "."
       << HEBENCH_API_VERSION_MINOR << "."
       << HEBENCH_API_VERSION_REVISION << "-"
       << HEBENCH_API_VERSION_BUILD << std::endl;
    return os;
}

void initArgsParser(hebench::ArgsParser &parser, int argc, char **argv)
{
    parser.addArgument("--backend_lib_path", "--backend", "-b", 1, "<path_to_shared_lib>",
                       "   [REQUIRED] Path to backend shared library.\n"
                       "   The library file must exist and be accessible for reading.");
    parser.addArgument("--benchmark_config_file", "--config_file", "-c", 1, "<path_to_config_file>",
                       "   [OPTIONAL] Path to benchmark run configuration file.\n"
                       "   YAML file specifying the selection of benchmarks and their workload\n"
                       "   parameters to run. If not present, all backend benchmarks will be run\n"
                       "   with default parameters.");
    parser.addArgument("--compile_reports", "--compile", "-C", 1, "<bool: 0|false|1|true>",
                       "   [OPTIONAL] Specifies whether summaries and statistics should be compiled\n"
                       "   from the reports generated by the benchmarks (TRUE) or not (FALSE).\n"
                       "   Extracting statistics may be time consuming, depending directly on the\n"
                       "   number of events recorded in each report. Defaults to \"TRUE\".");
    parser.addArgument("--dump_config", "--dump", 0, "",
                       "   [OPTIONAL] If specified, Test Harness will dump a general configuration\n"
                       "   file with the possible benchmarks that the backend can run. This file can\n"
                       "   be used as starting point template for a benchmark run configuration file.\n"
                       "   The destination file is specified by \"--benchmark_config_file\" argument.");
    parser.addArgument("--enable_validation", "--validation", "-v", 1, "<bool: 0|false|1|true>",
                       "   [OPTIONAL] Specifies whether results from benchmarks ran will be validated\n"
                       "   against ground truth. Defaults to \"TRUE\".");
    parser.addArgument("--run_overview", 1, "<bool: 0|false|1|true>",
                       "   [OPTIONAL] Specifies whether final summary overview of the benchmarks ran\n"
                       "   will be printed in standard output (TRUE) or not (FALSE). Results of the\n"
                       "   run will always be saved to storage regardless. Defaults to \"TRUE\".");
    parser.addArgument("--random_seed", "--seed", 1, "<uint64>",
                       "   [OPTIONAL] Specifies the random seed to use for pseudo-random number\n"
                       "   generation when none is specified by a benchmark configuration file. If\n"
                       "   no seed is specified, the current system clock time will be used as seed.");
    parser.addArgument("--report_delay", 1, "<delay_in_ms>",
                       "   [OPTIONAL] Delay between progress reports. Before each benchmark starts,\n"
                       "   Test Harness will pause for this specified number of milliseconds.\n"
                       "   Pass 0 to avoid delays. Defaults to 1000 ms.");
    parser.addArgument("--report_root_path", "--output_dir", 1, "<path_to_directory>",
                       "   [OPTIONAL] Directory where to store the report output files.\n"
                       "   Must exist and be accessible for writing. Any files with the same name will\n"
                       "   be overwritten. Defaults to current working directory \".\"");
    parser.addArgument("--single_path_report", "--single_path", 0, "",
                       "   [OPTIONAL] Allows the user to choose if the benchmark's report (s) will be\n"
                       "   created in a single-level directory or not.");
    parser.addArgument("--version", 0, "",
                       "   [OPTIONAL] Outputs Test Harness version, required API Bridge version and\n"
                       "   currently linked API Bridge version. Application exits after this.");
    parser.parse(argc, argv);
}

void generateOverview(std::ostream &os,
                      const std::vector<std::string> &report_paths,
                      const std::string &input_root_path,
                      bool b_single_path_reports)
{
    // Generates a condensed, pretty print version summarizing each benchmark result.

    constexpr int ScreenColSize    = 80;
    constexpr int AveWallColSize   = ScreenColSize / 8;
    constexpr int AveCPUColSize    = ScreenColSize / 8;
    constexpr int BenchNameColSize = ScreenColSize - AveWallColSize - AveCPUColSize - 15;

    std::stringstream ss;

    os << " " << std::setfill(' ') << std::setw(BenchNameColSize) << std::left << std::string("Benchmark").substr(0, BenchNameColSize) << " | "
       << std::setw(AveWallColSize + 3) << std::right << std::string("Ave Wall time").substr(0, AveWallColSize + 3) << " | "
       << std::setw(AveWallColSize + 3) << std::right << std::string("Ave CPU time").substr(0, AveCPUColSize + 3) << std::endl;
    os << std::setfill('=') << std::setw(ScreenColSize) << std::left << "=" << std::endl;

    for (std::size_t report_i = 0; report_i < report_paths.size(); ++report_i)
    {
        // retrieve the correct input and output paths
        std::filesystem::path report_location = report_paths[report_i]; // path where report was saved
        std::filesystem::path report_path;

        if (report_location.is_absolute())
            report_path = report_location;
        else
            report_path = std::filesystem::canonical(input_root_path) / report_location;

        if (b_single_path_reports)
            report_path += (hebench::TestHarness::hyphen + std::string(hebench::TestHarness::FileNameNoExtReport));
        else
            report_path /= hebench::TestHarness::FileNameNoExtReport;
        // Adding the file extension
        report_path += ".csv";

        ss = std::stringstream();
        ss << (report_i + 1) << ". " << report_location.generic_string();
        os << " " << std::setfill(' ') << std::setw(BenchNameColSize) << std::left << ss.str().substr(0, BenchNameColSize) << " | ";

        try
        {
            // load input report
            hebench::ReportGen::cpp::TimingReport report =
                hebench::ReportGen::cpp::TimingReport::loadReportFromCSVFile(report_path);
            // generate summary
            if (report.getEventCount() > 0)
            {
                // compute simple stats on the main event for this report

                hebench::Utilities::Math::EventStats stats_wall;
                hebench::Utilities::Math::EventStats stats_cpu;

                for (std::uint64_t i = 0; i < report.getEventCount(); ++i)
                {
                    hebench::ReportGen::TimingReportEventC event;
                    report.getEvent(event, i);
                    if (event.event_type_id == report.getMainEventType())
                    {
                        double wall_time = hebench::ReportGen::cpp::TimingReport::computeElapsedWallTime(event) / event.iterations;
                        double cpu_time  = hebench::ReportGen::cpp::TimingReport::computeElapsedCPUTime(event) / event.iterations;
                        for (std::uint64_t i = 0; i < event.iterations; ++i)
                        {
                            stats_wall.newEvent(wall_time);
                            stats_cpu.newEvent(cpu_time);
                        } // end for
                    } // end if
                } // end for

                // output overview of summary to stdout
                hebench::ReportGen::TimingPrefixedSeconds timing_prefix;
                double elapsed_time_secs;
                std::string s_elapsed_time;

                // wall time average

                elapsed_time_secs = stats_wall.getMean();
                // convert to timing prefix that fits the value between 1 and 1000
                hebench::ReportGen::cpp::TimingPrefixUtility::computeTimingPrefix(timing_prefix, elapsed_time_secs);
                ss = std::stringstream();
                ss << timing_prefix.symbol << "s";
                // convert to string with, at most, 2 decimal places
                s_elapsed_time = hebench::Utilities::convertDoubleToStr(timing_prefix.value, 2);
                // if string doesn't fit in the column, attempt to use scientific notation
                if (timing_prefix.value < 0.1 || s_elapsed_time.size() > AveWallColSize)
                    s_elapsed_time = hebench::Utilities::convertDoubleToStrScientific(timing_prefix.value, AveWallColSize);
                // output value
                os << std::setw(AveWallColSize) << std::right
                   << s_elapsed_time
                   << std::setfill(' ') << std::setw(3) << std::right << ss.str() << " | ";

                // cpu time average

                elapsed_time_secs = stats_cpu.getMean();
                // convert to timing prefix that fits the value between 1 and 1000
                hebench::ReportGen::cpp::TimingPrefixUtility::computeTimingPrefix(timing_prefix, elapsed_time_secs);
                ss = std::stringstream();
                ss << timing_prefix.symbol << "s";
                // convert to string with, at most, 2 decimal places
                s_elapsed_time = hebench::Utilities::convertDoubleToStr(timing_prefix.value, 2);
                // if string doesn't fit in the column, attempt to use scientific notation
                if (timing_prefix.value < 0.1 || s_elapsed_time.size() > AveCPUColSize)
                    s_elapsed_time = hebench::Utilities::convertDoubleToStrScientific(timing_prefix.value, AveCPUColSize);
                // output value
                os << std::setw(AveCPUColSize) << std::right
                   << s_elapsed_time
                   << std::setfill(' ') << std::setw(3) << std::right << ss.str() << std::endl;
            } // end if
            else
                os << "Validation Failed" << std::endl;
        }
        catch (...)
        {
            os << "Load Failed" << std::endl;
        }
        os << std::setfill('-') << std::setw(ScreenColSize) << std::left << "-" << std::endl;
    } // end for
}

int main(int argc, char **argv)
{
    int retval = 0;
    ProgramConfig config;
    std::stringstream ss;

    std::cout << std::endl
              << hebench::Logging::GlobalLogger::log(true, "HEBench") << std::endl;

    std::vector<hebench::Utilities::BenchmarkRequest> benchmarks_to_run;
    std::size_t total_runs = 0;
    std::vector<std::string> report_paths;
    std::vector<std::size_t> failed_benchmarks;

    try
    {
        hebench::ArgsParser args_parser;
        initArgsParser(args_parser, argc, argv);
        config.initializeConfig(args_parser);

        ss = std::stringstream();
        config.showVersion(ss);
        std::cout << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

        if (HEBENCH_TEST_HARNESS_API_REQUIRED_VERSION_MAJOR != HEBENCH_API_VERSION_MAJOR
            || HEBENCH_TEST_HARNESS_API_REQUIRED_VERSION_MINOR != HEBENCH_API_VERSION_MINOR
            || HEBENCH_TEST_HARNESS_API_MIN_REQUIRED_VERSION_REVISION > HEBENCH_API_VERSION_REVISION)
        {
            throw std::runtime_error("Invalid API Bridge version.");
        } // end if

        ss = std::stringstream();
        config.showConfig(ss);
        std::cout << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

        ss = std::stringstream();
        ss << "Initializing Backend from shared library:" << std::endl
           << config.backend_lib_path;
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
        hebench::APIBridge::DynamicLibLoad::loadLibrary(config.backend_lib_path);
        std::cout << IOS_MSG_OK << hebench::Logging::GlobalLogger::log("Backend loaded successfully.") << std::endl;

        // create engine and register all benchmarks
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Initializing Backend engine...") << std::endl;
        hebench::TestHarness::Engine::Ptr p_engine = hebench::TestHarness::Engine::create();
        std::cout << IOS_MSG_OK << std::endl;

        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Retrieving default benchmark configuration from Backend...") << std::endl;
        std::shared_ptr<hebench::Utilities::BenchmarkConfigurator> p_bench_config =
            std::make_shared<hebench::Utilities::BenchmarkConfigurator>(p_engine, config.backend_lib_path);
        std::cout << IOS_MSG_DONE << std::endl;

        // default configuration for benchmarks

        if (config.b_dump_config)
        {
            ss = std::stringstream();
            ss << "Saving default benchmark configuration to storage:" << std::endl
               << config.config_file;
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
            p_bench_config->saveConfiguration(config.config_file,
                                              p_bench_config->getDefaultConfiguration(),
                                              config.random_seed);
            std::cout << IOS_MSG_OK << std::endl;

            // default config dumped; program completed
        } // end if
        else
        {
            // initialize benchmarks requested to run

            if (config.config_file.empty())
            {
                std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading default benchmark configuration...") << std::endl;
                benchmarks_to_run = p_bench_config->getDefaultConfiguration();
            } // end if
            else
            {
                ss = std::stringstream();
                ss << "Loading benchmark configuration file:" << std::endl
                   << config.config_file;
                std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
                benchmarks_to_run = p_bench_config->loadConfiguration(config.config_file, config.random_seed);
            } // end else
        } // end else

        p_bench_config.reset(); // clean up benchmark configurator

        if (!benchmarks_to_run.empty())
        {
            // start benchmarking if there are benchmarks to run

            ss = std::stringstream();
            config.showBenchmarkDefaults(ss);
            std::cout << std::endl
                      << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

            hebench::Utilities::RandomGenerator::setRandomSeed(config.random_seed);

            total_runs = benchmarks_to_run.size();
            ss         = std::stringstream();
            ss << "Benchmarks to run: " << total_runs;
            std::cout << IOS_MSG_OK << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

            // iterate through the registered benchmarks and execute them
            std::size_t run_i = 0;
            for (std::size_t bench_i = 0; bench_i < benchmarks_to_run.size(); ++bench_i)
            {
                benchmarks_to_run[bench_i].configuration.b_single_path_report = config.b_single_path_report;
                hebench::Utilities::BenchmarkRequest &benchmark_request       = benchmarks_to_run[bench_i];
                bool b_critical_error                                         = false;
                std::string bench_path;
                hebench::Utilities::TimingReportEx report;
                try
                {
                    ss = std::stringstream();
                    ss << " Progress: " << (run_i * 100 / total_runs) << "%" << std::endl
                       << "           " << run_i << "/" << total_runs;
                    std::cout << std::endl
                              << "==================" << std::endl
                              << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl
                              << "==================" << std::endl;

                    if (config.report_delay_ms > 0)
                        std::this_thread::sleep_for(std::chrono::milliseconds(config.report_delay_ms));

                    // obtain the text description of the benchmark to print out
                    hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr bench_token =
                        p_engine->describeBenchmark(benchmark_request.index, benchmark_request.configuration);

                    bench_path = bench_token->getDescription().path; //description.path;

                    // print header

                    // prints
                    // ===========================
                    //  Workload: <workload name>
                    // ===========================
                    std::string s_workload_name = "Workload: " + bench_token->getDescription().workload_name;
                    std::size_t fill_size       = s_workload_name.length() + 2;
                    if (fill_size > 79)
                        fill_size = 79;
                    std::cout << std::endl
                              << std::setfill('=') << std::setw(fill_size) << "=" << std::endl
                              << " " << hebench::Logging::GlobalLogger::log(s_workload_name) << std::endl
                              << std::setw(fill_size) << "=" << std::setfill(' ') << std::endl;

                    report.setHeader(bench_token->getDescription().header);
                    if (!bench_token->getBenchmarkConfiguration().dataset_filename.empty())
                    {
                        std::stringstream ss;
                        ss << "Dataset, \"" << bench_token->getBenchmarkConfiguration().dataset_filename << "\"" << std::endl;
                        report.appendFooter(ss.str());
                    } // end if

                    std::cout << std::endl
                              << report.getHeader() << std::endl;

                    // create the benchmark
                    hebench::TestHarness::IBenchmark::Ptr p_bench = p_engine->createBenchmark(bench_token, report);

                    hebench::TestHarness::IBenchmark::RunConfig run_config;
                    run_config.b_validate_results = config.b_validate_results;

                    // run the workload
                    bool b_succeeded = p_bench->run(report, run_config);

                    if (!b_succeeded)
                    {
                        std::cout << IOS_MSG_FAILED << hebench::Logging::GlobalLogger::log(bench_token->getDescription().workload_name) << std::endl;
                        failed_benchmarks.push_back(report_paths.size());
                        report.clear(); // report event data is no longer valid for a failed run
                    } // end if
                }
                catch (hebench::Common::ErrorException &err_num)
                {
                    if (err_num.getErrorCode() == HEBENCH_ECODE_CRITICAL_ERROR)
                    {
                        b_critical_error = true;
                        throw; // critical failure
                    } // end if
                    else
                    {
                        // no critical error: report and move on to the next benchmark

                        b_critical_error = false;

                        failed_benchmarks.push_back(report_paths.size());
                        report.clear(); // report event data is no longer valid for a failed run

                        ss = std::stringstream();
                        ss << "Workload backend failed with message: " << std::endl
                           << err_num.what();
                        std::cout << std::endl
                                  << IOS_MSG_ERROR << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
                    } // end else
                }
                catch (...)
                {
                    b_critical_error = true;
                    throw; // critical failure
                }

                report_paths.emplace_back(bench_path);

                // create the path to output report
                std::filesystem::path report_filename = bench_path;
                std::filesystem::path report_path     = report_filename.is_absolute() ?
                                                        report_filename :
                                                        config.report_root_path / report_filename;

                // output CSV report
                report_filename = report_path;
                if (!config.b_single_path_report)
                    report_filename /= hebench::TestHarness::FileNameNoExtReport;
                else
                    report_filename += (hebench::TestHarness::hyphen + std::string(hebench::TestHarness::FileNameNoExtReport));
                report_filename += ".csv";

                ss = std::stringstream();
                ss << "Saving report to: " << std::endl
                   << report_filename;
                std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

                if (b_critical_error)
                {
                    // delete any previous report in this location to signal failure
                    if (std::filesystem::exists(report_filename) && std::filesystem::is_regular_file(report_filename))
                    {
                        std::filesystem::remove(report_filename);
                    }
                } // end if
                else
                {
                    // no need to create dirs if single path is enabled
                    if (!config.b_single_path_report)
                    {
                        std::filesystem::create_directories(report_path);
                    }
                    report.save2CSV(report_filename);
                } // end else

                std::cout << IOS_MSG_OK << hebench::Logging::GlobalLogger::log("Report saved.") << std::endl;

                ++run_i;

                // benchmark cleaned up here automatically
            } // end for

            // clean-up engine before final report (engine can clean up
            // automatically, but better to release when no longer needed)
            p_engine.reset();

            // At this point all benchmarks have been run and reports generated.
            // All that's left to do is to perform output that summarizes the run.

            assert(report_paths.size() == total_runs);

            ss = std::stringstream();
            ss << " Progress: 100%" << std::endl
               << "           " << total_runs << "/" << total_runs;
            std::cout << std::endl
                      << "==================" << std::endl
                      << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl
                      << "==================" << std::endl;

            // benchmark summary list

            std::cout << std::endl
                      << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating benchmark list...") << std::endl;

            std::filesystem::path benchmark_list_filename = std::filesystem::canonical(config.report_root_path);
            benchmark_list_filename /= "benchmark_list.txt";
            hebench::Utilities::writeToFile(
                benchmark_list_filename,
                [&config, &report_paths](std::ostream &os) {
                    for (std::size_t report_i = 0; report_i < report_paths.size(); ++report_i)
                    {
                        std::filesystem::path report_filename = report_paths[report_i];
                        if (!config.b_single_path_report)
                            report_filename /= hebench::TestHarness::FileNameNoExtReport;
                        else
                            report_filename += (hebench::TestHarness::hyphen + std::string(hebench::TestHarness::FileNameNoExtReport));
                        report_filename += ".csv";

                        os << report_filename.string() << std::endl;
                    } // end for
                },
                false, false);

            ss = std::stringstream();
            ss << "Benchmark list saved to: " << std::endl
               << benchmark_list_filename;
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

            // compile reports into stats and summary files

            if (config.b_compile_reports)
            {
                std::cout << std::endl
                          << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Initializing report compiler...") << std::endl;

                hebench::ReportGen::Compiler::ReportCompilerConfigC compiler_config;
                std::vector<char> c_error_msg(1024, 0);
                std::string compile_filename       = benchmark_list_filename.string();
                compiler_config.input_file         = compile_filename.c_str();
                compiler_config.b_show_overview    = 0; // do not show the overview result file here
                compiler_config.b_silent           = 0;
                compiler_config.time_unit          = 0;
                compiler_config.time_unit_stats    = 0;
                compiler_config.time_unit_overview = 0;
                compiler_config.time_unit_summary  = 0;

                std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Compiling reports using default compiler options...") << std::endl
                          << std::endl;
                if (!hebench::ReportGen::Compiler::compile(&compiler_config, c_error_msg.data(), c_error_msg.size()))
                    throw std::runtime_error(c_error_msg.data());
                std::cout << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log("Reports Compiled.") << std::endl
                          << std::endl;
            } // end if

            // display overview of the run results

            if (config.b_show_run_overview)
            {
                std::cout << std::endl
                          << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating overview...") << std::endl
                          << std::endl;
                generateOverview(std::cout, report_paths, config.report_root_path, config.b_single_path_report);

                // display any failed benchmarks

                ss = std::stringstream();
                ss << "Failed benchmarks: " << failed_benchmarks.size() << std::endl
                   << std::endl;
                for (std::size_t i = 0; i < failed_benchmarks.size(); ++i)
                {
                    if (i > 0)
                        ss << std::endl;
                    assert(failed_benchmarks[i] < report_paths.size());
                    ss << i + 1 << ". " << report_paths.at(failed_benchmarks[i]) << std::endl;
                } // end for
            } // end if
            std::cout << std::endl
                      << hebench::Logging::GlobalLogger::log(ss.str());

            // benchmark overall summary

            std::cout << std::endl
                      << "=================================" << std::endl
                      << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Run Summary") << std::endl;
            ss = std::stringstream();
            ss << "Total benchmarks run: " << total_runs;
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
            ss = std::stringstream();
            ss << "Success: " << total_runs - failed_benchmarks.size();
            if (!config.b_validate_results)
                ss << "* (validation skipped)";
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
            ss = std::stringstream();
            ss << "Failed: " << failed_benchmarks.size();
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
        } // end if
    }
    catch (hebench::ArgsParser::HelpShown &)
    {
        // do nothing
    }
    catch (std::exception &ex)
    {
        ss = std::stringstream();
        ss << "An error occurred with message: " << std::endl
           << ex.what();
        std::cout << std::endl
                  << IOS_MSG_ERROR << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
        retval = -1;
    }
    catch (...)
    {
        retval = -1;
    }

    hebench::APIBridge::DynamicLibLoad::unloadLibrary();

    if (retval == 0)
    {
        std::cout << std::endl
                  << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log(true, "Complete!") << std::endl;
    } // end if
    else
    {
        ss = std::stringstream();
        ss << "Terminated with errors. Exit code: " << retval;
        std::cout << std::endl
                  << IOS_MSG_FAILED << hebench::Logging::GlobalLogger::log(true, ss.str()) << std::endl;
    } // end else

    return retval;
}
