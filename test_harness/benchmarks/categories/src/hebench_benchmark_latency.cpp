
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "modules/timer/include/timer.h"

#include "hebench/api_bridge/api.h"
#include "include/hebench_engine.h"
#include "include/hebench_math_utils.h"

#include "../include/hebench_benchmark_latency.h"

namespace hebench {
namespace TestHarness {

BenchmarkLatency::BenchmarkLatency(std::shared_ptr<Engine> p_engine,
                                   const IBenchmarkDescription::DescriptionToken &description_token) :
    PartialBenchmarkCategory(p_engine, description_token)
{
}

BenchmarkLatency::~BenchmarkLatency()
{
}

bool BenchmarkLatency::run(hebench::Utilities::TimingReportEx &out_report,
                           IBenchmark::RunConfig &run_config)
{
    return run(out_report, getDataset(), m_benchmark_configuration, run_config);
}

bool BenchmarkLatency::run(hebench::Utilities::TimingReportEx &out_report,
                           IDataLoader::Ptr p_dataset,
                           const IBenchmarkDescription::BenchmarkConfig bench_config,
                           IBenchmark::RunConfig &run_config)
{
    std::cout << std::endl
              << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Starting test...") << std::endl;

    std::stringstream ss;
    hebench::Common::EventTimer<true> timer; // high precision
    std::uint32_t event_id = getEventIDNext();
    std::string event_name;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;

    // The following is simplified since latency test is predefined to have a
    // single sample for each parameter and result

    constexpr std::uint64_t batch_size = 1; // all batch sizes are 1 in latency test

    // prepare the parameters for encoding

    // constexpr int op_params_count = 2; // operation has 2 parameters

    // DataPack param_packs[op_params_count];
    // param_packs[0].p_buffers = p_raw_inputs_A;
    // param_packs[0].buffer_count = A_count;
    // param_packs[0].param_position = 0; // A is the first parameter, so, position 0
    // param_packs[1].p_buffers = p_raw_inputs_B;
    // param_packs[1].buffer_count = B_count;
    // param_packs[1].param_position = 1; // B is the second parameter, so, position 1

    // create the data packs
    std::vector<hebench::APIBridge::DataPack> param_packs(p_dataset->getParameterCount());
    for (std::size_t param_i = 0; param_i < param_packs.size(); ++param_i)
    {
        assert(p_dataset->getParameterData(param_i).param_position == param_i);
        param_packs[param_i].p_buffers      = p_dataset->getParameterData(param_i).p_buffers;
        param_packs[param_i].buffer_count   = batch_size;
        param_packs[param_i].param_position = param_i;
    } // end for

    // pack the parameters based on encrypted/plain

    // PackedData packed_parameters;
    // packed_parameters.p_data_packs = param_packs;
    // packed_parameters.pack_count = op_params_count;

    // separate param packs in encrypted/plain
    std::vector<hebench::APIBridge::PackedData> packed_parameters(2);
    std::vector<std::vector<hebench::APIBridge::DataPack>> packed_parameters_data_packs(packed_parameters.size());
    std::bitset<sizeof(std::uint32_t)> cipher_param_mask(m_descriptor.cipher_param_mask);
    for (std::size_t i = 0; i < param_packs.size(); ++i)
    {
        // determine if this parameter is encrypted
        if (cipher_param_mask.test(i)) // m_descriptor.cipher_param_mask & (1 << i)
            packed_parameters_data_packs.front().push_back(param_packs[i]);
        else
            packed_parameters_data_packs.back().push_back(param_packs[i]);
    } // end for

    // pack the data in API bridge format
    std::memset(packed_parameters.data(), 0, sizeof(hebench::APIBridge::PackedData) * packed_parameters.size());
    for (std::size_t i = 0; i < packed_parameters.size(); ++i)
    {
        if (!packed_parameters_data_packs[i].empty())
        {
            packed_parameters[i].p_data_packs = packed_parameters_data_packs[i].data();
            packed_parameters[i].pack_count   = packed_parameters_data_packs[i].size();
        } // end for
    } // end for

    // encode the raw parameters data

    // Handle h_encoded_inputs;
    // encode(h_benchmark, &packed_parameters, &h_encoded_inputs);

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Encoding.") << std::endl;

    std::vector<RAIIHandle> h_inputs(packed_parameters.size());
    for (std::size_t i = 0; i < packed_parameters.size(); ++i)
    {
        event_id = getEventIDNext();
        if (packed_parameters[i].pack_count > 0)
        {
            event_name = "Encoding pack " + std::to_string(i);
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(event_name + "...") << std::endl;
            timer.start();
            validateRetCode(hebench::APIBridge::encode(handle(), &packed_parameters[i], &h_inputs[i].handle));
            p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
            out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);
        } // end if
        else
            std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Pack " + std::to_string(i) + " is empty (skipping).") << std::endl;
    } // end for

    std::cout << IOS_MSG_OK << std::endl;

    // encrypt the encoded data

    event_id   = getEventIDNext();
    event_name = "Encryption";

    // Handle h_cipher_inputs;
    // encrypt(h_benchmark, h_encoded_inputs, &h_cipher_inputs);

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Encryption.") << std::endl;

    if (packed_parameters[0].pack_count > 0)
    {
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Encrypting...") << std::endl;

        hebench::APIBridge::Handle encrypted_input;
        // we have data to encrypt
        timer.start();
        validateRetCode(hebench::APIBridge::encrypt(handle(), h_inputs.front().handle, &encrypted_input));
        p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
        out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

        // overwrite the first input handle by its encrypted version
        h_inputs.front() = encrypted_input; // old handle automatically destroyed by RAII

        std::cout << IOS_MSG_OK << std::endl;
    } // end if
    else
        std::cout << IOS_MSG_WARNING << hebench::Logging::GlobalLogger::log("No encrypted parameters requested (skipping).") << std::endl;

    // load encrypted data into backend's remote to use as input to the operation

    event_id   = getEventIDNext();
    event_name = "Loading";

    // Handle h_remote_inputs;
    // load(h_benchmark,
    //      &h_cipher_inputs, 1, // only 1 PackedData
    //      &h_remote_inputs);

    // prepare handles for loading
    std::vector<hebench::APIBridge::Handle> h_inputs_local;
    for (std::size_t i = 0; i < packed_parameters.size(); ++i)
    {
        if (packed_parameters[i].pack_count > 0)
            h_inputs_local.push_back(h_inputs[i].handle);
    } // end for

    // load handles

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading data to remote backend...") << std::endl;

    RAIIHandle h_inputs_remote;
    timer.start();
    validateRetCode(hebench::APIBridge::load(handle(),
                                             h_inputs_local.data(), h_inputs_local.size(),
                                             &h_inputs_remote.handle));
    p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

    std::cout << IOS_MSG_OK << std::endl;

    // clean up data we no longer need (in reverse order of creation)

    // destroyHandle(h_cipher_inputs);
    // destroyHandle(h_encoded_inputs);

    h_inputs_local.clear();
    h_inputs.clear(); // input handles cleaned up automatically here
    packed_parameters.clear();
    packed_parameters_data_packs.clear();
    param_packs.clear();

    // prepare parameter indexers for operation

    // ParameterIndexer params[op_params_count]; // one indexer per parameter
    // params[0].batch_size = 1;  // for parameter A we will be using one value
    // params[0].value_index = 0; // starting with the first value

    // params[1].batch_size = 10; // for parameter B we will be using 10 values
    // params[1].value_index = 0; // starting with the first.

    std::vector<hebench::APIBridge::ParameterIndexer> params(p_dataset->getParameterCount());
    for (std::size_t i = 0; i < params.size(); ++i)
    {
        params[i].batch_size  = batch_size;
        params[i].value_index = 0;
    } // end if

    // operate

    event_id   = getEventIDNext();
    event_name = "Warmup";

    // Handle h_remote_result;
    // operate(h_benchmark,
    //         h_remote_inputs, params,
    //         &h_remote_result);

    if (m_descriptor.cat_params.latency.warmup_iterations_count > 0)
    {
        std::cout << IOS_MSG_INFO
                  << hebench::Logging::GlobalLogger::log("Starting warm-up iterations: requested "
                                                         + std::to_string(m_descriptor.cat_params.latency.warmup_iterations_count)
                                                         + " iterations.")
                  << std::endl;

        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Warming up...") << std::endl;

        // warm up
        for (std::uint64_t rep_i = 0; rep_i < m_descriptor.cat_params.latency.warmup_iterations_count; ++rep_i)
        {
            RAIIHandle h_result_remote;
            timer.start();
            validateRetCode(hebench::APIBridge::operate(handle(),
                                                        h_inputs_remote.handle, params.data(),
                                                        &h_result_remote.handle));
            p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
            out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);
        } // end for

        std::cout << IOS_MSG_DONE << std::endl;
    } // end if
    else
    {
        std::cout << IOS_MSG_WARNING
                  << hebench::Logging::GlobalLogger::log("No warm-up requested (skipping).")
                  << std::endl;
    } // end else

    std::uint64_t min_test_time_ms = m_descriptor.cat_params.latency.min_test_time_ms > 0 ?
                                         m_descriptor.cat_params.latency.min_test_time_ms :
                                         bench_config.default_min_test_time_ms;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Starting latency test.") << std::endl
              << std::string(sizeof(IOS_MSG_INFO) + 1, ' ') << hebench::Logging::GlobalLogger::log("Requested time: " + std::to_string(m_descriptor.cat_params.latency.min_test_time_ms) + " ms") << std::endl
              << std::string(sizeof(IOS_MSG_INFO) + 1, ' ') << hebench::Logging::GlobalLogger::log("Actual time: " + std::to_string(min_test_time_ms) + " ms") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Testing...") << std::endl;

    event_id   = getEventIDNext();
    event_name = "Operation";

    out_report.addEventType(event_id, event_name, true);

    // measure the operation after warm up
    std::vector<RAIIHandle> h_remote_results;
    h_remote_results.reserve(20); // initial capacity for 20 iterations
    out_report.setEventCapacity(out_report.getEventCapacity() + h_remote_results.capacity());
    std::uint64_t op_count = 0;
    double elapsed_ms      = 0.0;
    while (op_count < 2 || elapsed_ms < min_test_time_ms)
    {
        hebench::APIBridge::Handle h_result_remote;
        timer.start();
        validateRetCode(hebench::APIBridge::operate(handle(),
                                                    h_inputs_remote.handle, params.data(),
                                                    &h_result_remote));
        p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
        elapsed_ms += p_timing_event->elapsedWallTime<std::milli>();
        // check if we have enough capacity
        if (h_remote_results.capacity() == h_remote_results.size()
            && elapsed_ms > 0.0)
        {
            // capacity exceeded: over estimate capacity needed for the whole operation
            // so that it is less likely that we need to reallocate again
            std::size_t tmp_multiplier = static_cast<std::size_t>(min_test_time_ms / elapsed_ms);
            std::size_t max_capacity   = h_remote_results.capacity()
                                       * (tmp_multiplier + (tmp_multiplier > 0 ? 1 : 2));
            out_report.setEventCapacity(out_report.getEventCapacity()
                                        + (max_capacity - h_remote_results.capacity()));
            h_remote_results.reserve(max_capacity);
        } // end if
        out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);
        h_remote_results.emplace_back(h_result_remote);

        ++op_count;
    } // end while

    std::cout << IOS_MSG_DONE << std::endl;

    // clean up data we no longer need

    // destroyHandle(h_remote_inputs);

    h_inputs_remote.destroy();

    // postprocess output

    // Handle h_cipher_output;

    // retrieve data from backend's remote and store in host

    event_id   = getEventIDNext();
    event_name = "Store";

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Retrieving data from remote backend...") << std::endl;

    std::vector<RAIIHandle> h_cipher_results(h_remote_results.size());
    for (std::size_t i = 0; i < h_remote_results.size(); ++i)
    {
        // store(h_benchmark, h_remote_result,
        //       &h_cipher_output, 1 // Only 1 local PackedData for result expected for this operation.
        //      );

        timer.start();
        validateRetCode(hebench::APIBridge::store(handle(),
                                                  h_remote_results[i].handle,
                                                  &h_cipher_results[i].handle,
                                                  1));
        p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
        out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

        // clean up data we no longer need
        // destroyHandle(h_remote_result);

        // even though it is RAII, it is better to free up space on remote manually here,
        // just in case it is a device with low memory capacity
        h_remote_results[i].destroy();
    } // end for
    h_remote_results.clear();

    std::cout << IOS_MSG_OK << std::endl;

    // decrypt results

    event_id   = getEventIDNext();
    event_name = "Decryption";

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Decrypting results...") << std::endl;

    std::vector<RAIIHandle> h_plain_results(h_cipher_results.size());
    for (std::size_t i = 0; i < h_cipher_results.size(); ++i)
    {
        // Handle h_plain_result;
        // decrypt(h_benchmark, h_cipher_output, &h_plain_result);

        timer.start();
        validateRetCode(hebench::APIBridge::decrypt(handle(), h_cipher_results[i].handle, &h_plain_results[i].handle));
        p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
        out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

        // // clean up data we no longer need
        // destroyHandle(h_cipher_output);

        // even though it is RAII, it is better to free up space here,
        // just in case these local handles are large
        h_cipher_results[i].destroy();
    } // end for
    h_cipher_results.clear();

    std::cout << IOS_MSG_OK << std::endl;

    // allocate space for decoded results

    // NativeDataBuffer p_raw_results[?]; // allocate space for all outputs

    //    std::vector<std::uint8_t> raw_result_buffer(p_dataset->getResultData(0).p_buffers[0].size);
    //    hebench::APIBridge::NativeDataBuffer raw_results;
    //    raw_results.p    = raw_result_buffer.data();
    //    raw_results.size = raw_result_buffer.size();
    //    raw_results.tag  = 0;

    // allocate space for all outputs

    std::vector<std::uint8_t> raw_result_buffer;
    std::uint64_t max_raw_result_size = 0;
    for (std::uint64_t result_pos = 0; result_pos < p_dataset->getResultCount(); ++result_pos)
        max_raw_result_size += p_dataset->getResultData(result_pos).p_buffers[0].size;
    raw_result_buffer.resize(max_raw_result_size);

    // point to the allocated buffers

    std::vector<hebench::APIBridge::NativeDataBuffer> raw_results(p_dataset->getResultCount(),
                                                                  hebench::APIBridge::NativeDataBuffer({ 0, 0, 0 }));
    std::uint64_t offset_p = 0;
    for (std::uint64_t result_pos = 0; result_pos < p_dataset->getResultCount(); ++result_pos)
    {
        if (p_dataset->getResultData(result_pos).buffer_count > 0)
        {
            raw_results[result_pos].p    = raw_result_buffer.data() + offset_p;
            raw_results[result_pos].size = p_dataset->getResultData(result_pos).p_buffers[0].size;
            raw_results[result_pos].tag  = 0;

            offset_p += raw_results[result_pos].size;
        } // end if
    } // end for

    // DataPack results_pack;
    // results_pack.p_buffers = p_raw_results;
    // results_pack.buffer_count = A_count// B_count;
    // results_pack.param_position = 0; // we are retrieving results in the first position into this data pack

    std::vector<hebench::APIBridge::DataPack> results_pack(p_dataset->getResultCount());
    for (std::size_t result_pos = 0; result_pos < results_pack.size(); ++result_pos)
    {
        results_pack[result_pos].p_buffers      = &raw_results[result_pos];
        results_pack[result_pos].buffer_count   = 1;
        results_pack[result_pos].param_position = 0;
    } // end for

    // PackedData packed_results;
    // packed_results.p_data_packs = &results_pack;
    // packed_results.pack_count = 1; // result shape is [1, 10]

    hebench::APIBridge::PackedData packed_results;
    packed_results.p_data_packs = results_pack.data();
    packed_results.pack_count   = results_pack.size();

    // decode the results

    event_id   = getEventIDNext();
    event_name = "Decoding";

    if (run_config.b_validate_results)
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Decoding and Validation.") << std::endl;
    else
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Decoding...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Result", false);

    std::size_t mini_reports_cnt        = 3;
    std::size_t report_every_n_elements = h_plain_results.size() / 7 + 1;
    if (report_every_n_elements <= 0)
        report_every_n_elements = 1;
    std::size_t report_every_n_by_3_elements = report_every_n_elements / 4;
    if (report_every_n_by_3_elements <= 0)
        report_every_n_by_3_elements = 1;
    std::size_t progress_report_batch = 0;

    bool b_valid = true;
    for (std::size_t i = 0; i < h_plain_results.size(); ++i)
    {
        const auto &h_plain = h_plain_results[i].handle;
        if (b_valid)
        {
            if (i == 0 || (i + 1) % report_every_n_elements == 0)
            {
                // report operation to show sign of life
                while (mini_reports_cnt++ < 3)
                    std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
                mini_reports_cnt      = 0;
                progress_report_batch = 0;
                std::cout << hebench::Logging::GlobalLogger::log(" " + std::to_string(i + 1), false) << std::flush;
            } // end if

            // decode(Handle h_benchmark, h_plain_result, &packed_results);

            timer.start();
            validateRetCode(hebench::APIBridge::decode(handle(), h_plain, &packed_results));
            p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
            out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

            //            if (i + 1 < h_plain_results.size() && mini_reports_cnt < 3
            //                && (mini_reports_cnt == 0 || (i + 1) % report_every_n_by_3_elements == 0))
            if (mini_reports_cnt < 3
                && progress_report_batch > 0
                && progress_report_batch % report_every_n_by_3_elements == 0)
            {
                // report operation to show sign of life
                ++mini_reports_cnt;
                std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
            } // end if

            // validate output
            if (run_config.b_validate_results)
            {
                std::string s_error_msg;
                std::vector<std::uint64_t> data_pack_indices;
                std::vector<hebench::APIBridge::NativeDataBuffer *> outputs;
                try
                {
                    outputs.resize(p_dataset->getResultCount());
                    for (std::uint64_t i = 0; i < packed_results.pack_count; ++i)
                    {
                        if (packed_results.p_data_packs[i].param_position > outputs.size())
                            throw std::runtime_error("Invalid result position received from decoding.");
                        if (packed_results.p_data_packs[i].buffer_count <= 0)
                            throw std::runtime_error("Invalid empty result buffers received from decoding.");
                        outputs[packed_results.p_data_packs[i].param_position] =
                            &packed_results.p_data_packs[i].p_buffers[0];
                    } // end for

                    data_pack_indices.resize(p_dataset->getParameterCount(), 0);
                    b_valid = validateResult(p_dataset, data_pack_indices.data(),
                                             outputs,
                                             m_descriptor.data_type);
                }
                catch (std::exception &ex)
                {
                    b_valid     = false;
                    s_error_msg = ex.what();
                }
                catch (...)
                {
                    b_valid = false;
                }

                if (!b_valid)
                {
                    std::cout << hebench::Logging::GlobalLogger::log("", true) << std::endl;
                    ss = std::stringstream();
                    ss << "Validation failed" << std::endl
                       << "Result, " << i + 1 << std::endl;
                    if (!s_error_msg.empty())
                        ss << s_error_msg << std::endl;
                    std::cout << IOS_MSG_FAILED << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

                    // log the parameters, expected result and received result.
                    ss << std::endl;
                    logResult(ss, p_dataset, data_pack_indices.data(),
                              outputs,
                              m_descriptor.data_type);
                    out_report.appendFooter(ss.str());
                } // end else
            } // end if

            ++progress_report_batch;
        } // end if

        // clean up data we no longer need

        // even though it is RAII, it is better to free up space here,
        // just in case these local handles are large
        h_plain_results[i].destroy();
    } // end for
    h_plain_results.clear();

    if (b_valid)
    {
        // report valid op
        if ((op_count) % report_every_n_elements != 0)
        {
            // pretty-print end of result report
            while (mini_reports_cnt++ < 3)
                std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
            std::cout << hebench::Logging::GlobalLogger::log(" " + std::to_string(op_count)) << std::endl;
        } // end if
        else
            std::cout << hebench::Logging::GlobalLogger::log("") << std::endl;
        std::cout << IOS_MSG_OK << std::endl;
    } // end if

    if (!run_config.b_validate_results)
    {
        out_report.prependFooter("Validation skipped");
        std::cout << IOS_MSG_WARNING << hebench::Logging::GlobalLogger::log("Validation skipped.") << std::endl;
    } // end if

    std::cout << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log("Test Completed.") << std::endl;

    return b_valid;
}

} // namespace TestHarness
} // namespace hebench
