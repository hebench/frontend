
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "modules/args_parser/include/args_parser.h"
#include "modules/general/include/error.h"
#include "modules/logging/include/logging.h"

#include "dynamic_lib_load.h"

#include "include/hebench_config.h"
#include "include/hebench_engine.h"
#include "include/hebench_types_harness.h"
#include "include/hebench_utilities.h"
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
    std::uint64_t random_seed;
    std::size_t report_delay_ms;
    std::filesystem::path report_root_path;
    bool b_show_run_overview;

    static constexpr const char *DefaultConfigFile    = "";
    static constexpr std::uint64_t DefaultMinTestTime = 0;
    static constexpr std::uint64_t DefaultSampleSize  = 0;
    static constexpr std::size_t DefaultReportDelay   = 1000;
    static constexpr const char *DefaultRootPath      = ".";

    void initializeConfig(const hebench::ArgsParser &parser);
    static std::ostream &showBenchmarkDefaults(std::ostream &os, const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &bench_config);
    std::ostream &showConfig(std::ostream &os) const;
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
}

std::ostream &ProgramConfig::showBenchmarkDefaults(std::ostream &os, const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &bench_config)
{
    os << "Benchmark defaults:" << std::endl
       << "    Random seed: " << bench_config.random_seed << std::endl
       << "    Default minimum test time: " << bench_config.default_min_test_time_ms << " ms" << std::endl
       << "    Default sample size: " << bench_config.default_sample_size << std::endl;
    return os;
}

std::ostream &ProgramConfig::showConfig(std::ostream &os) const
{
    os << "Global Configuration:" << std::endl;
    if (b_dump_config)
    {
        os << "    Dumping configuration file!" << std::endl;
    } // end of
    else
    {
        os
            //<< "    Random seed: " << random_seed << std::endl
            << "    Validate results: " << (b_validate_results ? "Yes" : "No") << std::endl
            << "    Report delay (ms): " << report_delay_ms << std::endl
            << "    Report Root Path: " << report_root_path << std::endl
            << "    Show run overview: " << (b_show_run_overview ? "Yes" : "No") << std::endl
            //           << "    Benchmark defaults:" << std::endl
            //           << "        Default minimum test time: " << min_test_time_ms << " ms" << std::endl
            //           << "        Default sample size: " << default_sample_size << std::endl
            ;
    } // end if
    os << "    Run configuration file: ";
    if (config_file.empty())
        os << "(none)" << std::endl;
    else
        os << config_file << std::endl;
    os << "    Backend library: " << backend_lib_path << std::endl;
    return os;
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
    parser.addArgument("--version", 0, "",
                       "   [OPTIONAL] Outputs Test Harness version, required API Bridge version and\n"
                       "   currently linked API Bridge version. Application exists after this.");
    parser.parse(argc, argv);
}

std::string toDoubleVariableFrac(double x, int up_to_digits_after_dot)
{
    // Prints a floating point value with up to a number of digits after decimal point
    // with no trailing zeroes.
    std::string retval;
    std::stringstream ss;
    if (up_to_digits_after_dot < 0)
        up_to_digits_after_dot = 0;
    ss << std::fixed << std::setprecision(up_to_digits_after_dot) << x;
    retval = ss.str();
    retval.erase(retval.find_last_not_of(".0") + 1);
    return retval;
}

void generateSummary(const hebench::TestHarness::Engine &engine,
                     const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig bench_config,
                     const std::vector<hebench::TestHarness::BenchmarkRequest> &benchmarks_ran,
                     const std::string &input_root_path, const std::string &output_root_path,
                     bool do_stdout_summary = true)
{
    constexpr int ScreenColSize  = 80;
    constexpr int AveWallColSize = ScreenColSize / 8;
    constexpr int AveCPUColSize  = ScreenColSize / 8;
    //constexpr int BenchNameColSize = ScreenColSize * 9 / 16;
    constexpr int BenchNameColSize = ScreenColSize - AveWallColSize - AveCPUColSize - 15;

    std::stringstream ss;

    if (do_stdout_summary)
    {
        std::cout << " " << std::setfill(' ') << std::setw(BenchNameColSize) << std::left << std::string("Benchmark").substr(0, BenchNameColSize) << " | "
                  << std::setw(AveWallColSize + 3) << std::right << std::string("Ave Wall time").substr(0, AveWallColSize + 3) << " | "
                  << std::setw(AveWallColSize + 3) << std::right << std::string("Ave CPU time").substr(0, AveCPUColSize + 3) << std::endl;
        std::cout << std::setfill('=') << std::setw(ScreenColSize) << std::left << "=" << std::endl;
    } // end if

    std::size_t bench_total = 0;
    for (std::size_t bench_i = 0; bench_i < benchmarks_ran.size(); ++bench_i)
    {
        for (std::size_t params_i = 0; params_i < benchmarks_ran[bench_i].sets_w_params.size(); ++params_i)
        {
            hebench::TestHarness::BenchmarkFactory::BenchmarkToken::Ptr description_token =
                engine.describeBenchmark(bench_config,
                                         benchmarks_ran[bench_i].benchmark_index,
                                         benchmarks_ran[bench_i].sets_w_params[params_i]);

            // retrieve the correct input and output paths
            std::filesystem::path report_filename = description_token->description.path;
            std::filesystem::path report_path;
            std::filesystem::path output_path;

            if (report_filename.is_absolute())
            {
                report_path = report_filename;
                output_path = report_filename;
            } // end if
            else
            {
                report_path = input_root_path / report_filename;
                output_path = output_root_path / report_filename;
            } // end else

            // generate output directory if it doesn't exits
            std::filesystem::create_directories(output_path);

            // complete the paths to the input and output files
            report_path /= hebench::TestHarness::FileNameNoExtReport;
            report_path += ".csv";
            output_path /= hebench::TestHarness::FileNameNoExtSummary;
            output_path += ".csv";

            if (do_stdout_summary)
            {
                ss = std::stringstream();
                ss << (bench_total + 1) << ". " << report_filename.generic_string();
                std::cout << " " << std::setfill(' ') << std::setw(BenchNameColSize) << std::left << ss.str().substr(0, BenchNameColSize) << " | ";
            } // end if

            try
            {
                // load input report
                hebench::TestHarness::Report::cpp::TimingReport report =
                    hebench::TestHarness::Report::cpp::TimingReport::loadReportFromCSVFile(report_path);
                // generate summary
                if (report.getEventCount() > 0)
                {
                    // output summary to file
                    hebench::TestHarness::Report::TimingReportEventC tre;
                    std::string csv_report = report.generateSummaryCSV(tre);
                    hebench::Utilities::writeToFile(output_path, csv_report.c_str(), csv_report.size(), false, false);

                    // output overview of summary to stdout
                    if (do_stdout_summary)
                    {
                        hebench::TestHarness::Report::TimingPrefixedSeconds timing_prefix;
                        double elapsed_time_secs;

                        elapsed_time_secs = (tre.wall_time_end - tre.wall_time_start) * tre.time_interval_ratio_num / tre.time_interval_ratio_den;
                        hebench::TestHarness::Report::cpp::TimingReport::computeTimingPrefix(timing_prefix, elapsed_time_secs);
                        ss = std::stringstream();
                        ss << timing_prefix.symbol << "s";
                        std::cout << std::setw(AveWallColSize) << std::right
                                  << toDoubleVariableFrac(timing_prefix.value, 2).substr(0, AveWallColSize)
                                  << std::setfill(' ') << std::setw(3) << std::right << ss.str() << " | ";

                        elapsed_time_secs = (tre.cpu_time_end - tre.cpu_time_start) * tre.time_interval_ratio_num / tre.time_interval_ratio_den;
                        hebench::TestHarness::Report::cpp::TimingReport::computeTimingPrefix(timing_prefix, elapsed_time_secs);
                        ss = std::stringstream();
                        ss << timing_prefix.symbol << "s";
                        std::cout << std::setw(AveCPUColSize) << std::right
                                  << toDoubleVariableFrac(timing_prefix.value, 2).substr(0, AveCPUColSize)
                                  << std::setfill(' ') << std::setw(3) << std::right << ss.str() << std::endl;
                    } // end if
                } // end if
                else if (do_stdout_summary)
                    std::cout << "Validation Failed" << std::endl;
            }
            catch (...)
            {
                if (do_stdout_summary)
                    std::cout << "Load Failed" << std::endl;
            }
            if (do_stdout_summary)
                std::cout << std::setfill('-') << std::setw(ScreenColSize) << std::left << "-" << std::endl;

            ++bench_total; // count the benchmark
        } // end for
    } // end for
}

void generateSummary(const hebench::TestHarness::Engine &engine,
                     const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig bench_config,
                     const std::vector<hebench::TestHarness::BenchmarkRequest> &benchmarks_ran,
                     const std::string &input_root_path,
                     bool do_stdout_summary = true)
{
    generateSummary(engine, bench_config, benchmarks_ran, input_root_path, input_root_path, do_stdout_summary);
}

int main(int argc, char **argv)
{
    int retval = 0;
    ProgramConfig config;
    std::stringstream ss;

    std::cout << std::endl
              << hebench::Logging::GlobalLogger::log(true, "HEBench") << std::endl;

    std::vector<hebench::TestHarness::BenchmarkRequest> benchmarks_to_run;
    std::size_t total_runs = 0;
    std::vector<std::string> failed_benchmarks;

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
        std::cout << IOS_MSG_OK << std::endl;

        // create engine and register all benchmarks
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Initializing Backend engine...") << std::endl;
        hebench::TestHarness::Engine::Ptr p_engine = hebench::TestHarness::Engine::create();
        std::cout << IOS_MSG_OK << std::endl;

        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Retrieving default benchmark configuration from Backend...") << std::endl;
        std::shared_ptr<hebench::Utilities::BenchmarkConfiguration> p_bench_config =
            std::make_shared<hebench::Utilities::BenchmarkConfiguration>(p_engine, config.backend_lib_path);
        std::cout << IOS_MSG_DONE << std::endl;

        // default configuration for benchmarks
        hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig bench_config;
        bench_config.default_sample_size      = ProgramConfig::DefaultSampleSize; //config.default_sample_size;
        bench_config.default_min_test_time_ms = ProgramConfig::DefaultMinTestTime; //config.min_test_time_ms;
        bench_config.random_seed              = config.random_seed;

        if (config.b_dump_config)
        {
            ss = std::stringstream();
            ss << "Saving default benchmark configuration to storage:" << std::endl
               << config.config_file;
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
            p_bench_config->saveConfiguration(config.config_file,
                                              p_bench_config->getDefaultConfiguration(),
                                              bench_config);
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
                benchmarks_to_run = p_bench_config->loadConfiguration(config.config_file, bench_config);
            } // end else

            ss = std::stringstream();
            config.showBenchmarkDefaults(ss, bench_config);
            std::cout << std::endl
                      << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

            hebench::Utilities::RandomGenerator::setRandomSeed(bench_config.random_seed);

            total_runs = hebench::Utilities::BenchmarkConfiguration::countBenchmarks2Run(benchmarks_to_run);
            ss         = std::stringstream();
            ss << "Benchmarks to run: " << total_runs;
            std::cout << IOS_MSG_OK << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

            // iterate through the registered benchmarks and execute them
            std::size_t run_i = 0;
            for (std::size_t bench_i = 0; bench_i < benchmarks_to_run.size(); ++bench_i)
            {
                for (std::size_t params_i = 0; params_i < benchmarks_to_run[bench_i].sets_w_params.size(); ++params_i)
                {
                    bool b_non_critical_error = false;
                    std::string bench_path;
                    hebench::Utilities::TimingReportEx report;
                    try
                    {
                        ss = std::stringstream();
                        ss << "(" << bench_i << ", " << params_i << ")";
                        bench_path = ss.str();

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
                        hebench::TestHarness::BenchmarkFactory::BenchmarkToken::Ptr bench_token =
                            p_engine->describeBenchmark(bench_config, benchmarks_to_run[bench_i].benchmark_index, benchmarks_to_run[bench_i].sets_w_params[params_i]);

                        bench_path = bench_token->description.path;

                        // print header

                        // prints
                        // ===========================
                        //  Workload: <workload name>
                        // ===========================
                        std::string s_workload_name = "Workload: " + bench_token->description.workload_name;
                        std::size_t fill_size       = s_workload_name.length() + 2;
                        if (fill_size > 79)
                            fill_size = 79;
                        std::cout << std::endl
                                  << std::setfill('=') << std::setw(fill_size) << "=" << std::endl
                                  << " " << hebench::Logging::GlobalLogger::log(s_workload_name) << std::endl
                                  << std::setw(fill_size) << "=" << std::setfill(' ') << std::endl;

                        std::cout << std::endl
                                  << bench_token->description.header << std::endl;

                        // create the benchmark
                        report.setHeader(bench_token->description.header);
                        hebench::TestHarness::IBenchmark::Ptr p_bench = p_engine->createBenchmark(bench_token, report);

                        hebench::TestHarness::IBenchmark::RunConfig run_config;
                        run_config.b_validate_results = config.b_validate_results;

                        // run the workload
                        bool b_succeeded = p_bench->run(report, run_config);

                        if (!b_succeeded)
                        {
                            std::cout << IOS_MSG_FAILED << hebench::Logging::GlobalLogger::log(bench_token->description.workload_name) << std::endl;
                            failed_benchmarks.push_back(bench_path);
                            report.clear(); // report event data is no longer valid for a failed run
                        } // end if
                    }
                    catch (hebench::Common::ErrorException &err_num)
                    {
                        if (err_num.getErrorCode() == HEBENCH_ECODE_CRITICAL_ERROR)
                            throw; // critical failure
                        else
                        {
                            // no critical error: report and move on to the next benchmark

                            b_non_critical_error = true;

                            failed_benchmarks.push_back(bench_path);
                            report.clear(); // report event data is no longer valid for a failed run

                            ss = std::stringstream();
                            ss << "Workload backend failed with message: " << std::endl
                               << err_num.what();
                            std::cout << std::endl
                                      << IOS_MSG_ERROR << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
                        } // end else
                    }

                    // create the path to output report
                    std::filesystem::path report_filename = bench_path;
                    std::filesystem::path report_path     = (report_filename.is_absolute() ?
                                                                 report_filename :
                                                                 config.report_root_path / report_filename);

                    ss = std::stringstream();
                    ss << "Saving report to: " << std::endl
                       << report_path;
                    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

                    // output CSV report
                    report_filename = report_path;
                    report_filename /= hebench::TestHarness::FileNameNoExtReport;
                    report_filename += ".csv";

                    if (b_non_critical_error)
                    {
                        // delete any previous report in this location to signal failure
                        if (std::filesystem::exists(report_filename)
                            && std::filesystem::is_regular_file(report_filename))
                            std::filesystem::remove(report_filename);
                    } // end if
                    else
                    {
                        std::filesystem::create_directories(report_path);
                        report.save2CSV(report_filename);
                    } // end else

                    std::cout << IOS_MSG_OK << hebench::Logging::GlobalLogger::log("Report saved.") << std::endl;

                    ++run_i;
                } // end for

                // benchmark cleaned up here automatically
            } // end for

            ss = std::stringstream();
            ss << " Progress: 100%" << std::endl
               << "           " << total_runs << "/" << total_runs;
            std::cout << std::endl
                      << "==================" << std::endl
                      << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl
                      << "==================" << std::endl;

            // benchmark summary

            std::cout << std::endl
                      << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating summary...") << std::endl
                      << std::endl;
            generateSummary(*p_engine, bench_config, benchmarks_to_run,
                            config.report_root_path, config.b_show_run_overview);

            // clean-up engine before final report (engine can clean up
            // automatically, but better to release when no longer needed)
            p_engine.reset();

            // benchmark overall summary

            if (config.b_show_run_overview && !failed_benchmarks.empty())
            {
                ss = std::stringstream();
                ss << "Failed benchmarks:" << std::endl
                   << std::endl;
                for (std::size_t i = 0; i < failed_benchmarks.size(); ++i)
                {
                    if (i > 0)
                        ss << std::endl;
                    ss << i + 1 << ". " << failed_benchmarks[i] << std::endl;
                } // end for
                std::cout << std::endl
                          << hebench::Logging::GlobalLogger::log(ss.str());
            } // end if

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
        } // end else
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
