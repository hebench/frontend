
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

#include "../include/hebench_benchmark_offline.h"

namespace hebench {
namespace TestHarness {

BenchmarkOffline::BenchmarkOffline(std::shared_ptr<Engine> p_engine,
                                   const IBenchmarkDescriptor::DescriptionToken &description_token) :
    PartialBenchmarkCategory(p_engine, description_token)
{
}

BenchmarkOffline::~BenchmarkOffline()
{
}

bool BenchmarkOffline::run(hebench::Utilities::TimingReportEx &out_report, RunConfig &config)
{
    return run(out_report, getDataset(), config);
}

bool BenchmarkOffline::run(hebench::Utilities::TimingReportEx &out_report,
                           IDataLoader::Ptr p_dataset,
                           IBenchmark::RunConfig &run_config)
{
    std::cout << std::endl
              << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Starting test...") << std::endl;

    std::stringstream ss;
    hebench::Common::EventTimer<true> timer; // high precision
    std::uint32_t event_id = getEventIDNext();
    std::string event_name;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;

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
        param_packs[param_i].buffer_count   = p_dataset->getParameterData(param_i).buffer_count;
        param_packs[param_i].param_position = param_i;
    } // end for

    // pack the parameters based on encrypted/plain

    // PackedData packed_parameters;
    // packed_parameters.p_data_packs = param_packs;
    // packed_parameters.pack_count = op_params_count;

    // separate param packs in encrypted/plain
    std::vector<hebench::APIBridge::PackedData> packed_parameters(2);
    std::vector<std::vector<hebench::APIBridge::DataPack>> packed_parameters_data_packs(packed_parameters.size());
    std::bitset<sizeof(std::uint32_t)> cipher_param_mask(this->getBackendDescription().descriptor.cipher_param_mask);
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
    std::uint64_t num_results = 1;
    for (std::size_t i = 0; i < params.size(); ++i)
    {
        params[i].batch_size  = p_dataset->getParameterData(i).buffer_count;
        params[i].value_index = 0;
        num_results *= params[i].batch_size;
    } // end if

    // operate

    std::uint64_t min_test_time_ms = this->getBenchmarkConfiguration().default_min_test_time_ms;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Starting offline test.") << std::endl
              << std::string(sizeof(IOS_MSG_INFO) + 1, ' ') << hebench::Logging::GlobalLogger::log("Minimum time: " + std::to_string(min_test_time_ms) + " ms") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Testing...") << std::endl;

    event_id   = getEventIDNext();
    event_name = "Operation";

    out_report.addEventType(event_id, event_name, true);

    // TODO: (nice to have) maybe have a way to report progress or check if backend is stuck
    // since this operation can be time consuming.

    RAIIHandle h_remote_results;
    std::size_t iteration_count    = 0;
    std::size_t iteration_capacity = 20; // initial capacity for 20 iterations
    double elapsed_ms              = 0.0;
    out_report.setEventCapacity(out_report.getEventCapacity() + iteration_capacity);
    while (iteration_count <= 0 || elapsed_ms < min_test_time_ms)
    {
        if (iteration_count > 0)
            // destroy previous result
            h_remote_results.destroy();
        timer.start();
        validateRetCode(hebench::APIBridge::operate(handle(),
                                                    h_inputs_remote.handle, params.data(),
                                                    &h_remote_results.handle));
        p_timing_event = timer.stop<DefaultTimeInterval>(event_id, num_results, nullptr);
        elapsed_ms += p_timing_event->elapsedWallTime<std::milli>();

        // check if we have enough capacity
        if (iteration_capacity == iteration_count
            && elapsed_ms > 0.0)
        {
            // capacity exceeded: over estimate capacity needed for the whole operation
            // so that it is less likely that we need to reallocate again
            std::size_t tmp_multiplier = static_cast<std::size_t>(min_test_time_ms / elapsed_ms);
            std::size_t max_capacity   = iteration_capacity
                                       * (tmp_multiplier + (tmp_multiplier > 0 ? 1 : 2));
            out_report.setEventCapacity(out_report.getEventCapacity()
                                        + (max_capacity - iteration_capacity));
            iteration_capacity = max_capacity;
        } // end if
        out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

        ++iteration_count;
    } // end while

    ss = std::stringstream();
    ss << "Elapsed time: " << p_timing_event->elapsedWallTime<std::milli>() << "ms";
    std::cout << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

    // clean up data we no longer need

    // destroyHandle(h_remote_inputs);

    h_inputs_remote.destroy();

    // postprocess output

    // Handle h_cipher_output;

    // retrieve data from backend's remote and store in host

    event_id   = getEventIDNext();
    event_name = "Store";

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Retrieving data from remote backend...") << std::endl;

    RAIIHandle h_cipher_results;
    // store(h_benchmark, h_remote_result,
    //       &h_cipher_output, 1 // Only 1 local PackedData for result expected for this operation.
    //      );

    timer.start();
    validateRetCode(hebench::APIBridge::store(handle(),
                                              h_remote_results.handle,
                                              &h_cipher_results.handle,
                                              1));
    p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

    // clean up data we no longer need
    // destroyHandle(h_remote_result);

    // even though it is RAII, it is better to free up space on remote manually here,
    // just in case it is a device with low memory capacity
    h_remote_results.destroy();

    std::cout << IOS_MSG_OK << std::endl;

    // decrypt results

    event_id   = getEventIDNext();
    event_name = "Decryption";

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Decrypting results...") << std::endl;

    RAIIHandle h_plain_results;
    // Handle h_plain_result;
    // decrypt(h_benchmark, h_cipher_output, &h_plain_result);

    timer.start();
    validateRetCode(hebench::APIBridge::decrypt(handle(), h_cipher_results.handle, &h_plain_results.handle));
    p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

    // // clean up data we no longer need
    // destroyHandle(h_cipher_output);

    // even though it is RAII, it is better to free up space here,
    // just in case these local handles are large
    h_cipher_results.destroy();

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
    //std::uint64_t num_buffers = 0;
    for (std::uint64_t result_component_i = 0; result_component_i < p_dataset->getResultCount(); ++result_component_i)
    {
        const hebench::APIBridge::DataPack &result_component = p_dataset->getResultData(result_component_i);
        for (std::uint64_t result_sample = 0; result_sample < result_component.buffer_count; ++result_sample)
            max_raw_result_size += result_component.p_buffers[result_sample].size;
    } // end for
    raw_result_buffer.resize(max_raw_result_size);

    // point to the allocated buffers

    std::vector<std::vector<hebench::APIBridge::NativeDataBuffer>> raw_results(p_dataset->getResultCount());
    std::uint64_t offset_p = 0;
    for (std::uint64_t result_component_i = 0; result_component_i < raw_results.size(); ++result_component_i)
    {
        const hebench::APIBridge::DataPack &result_component = p_dataset->getResultData(result_component_i);
        if (result_component.buffer_count > 0)
        {
            raw_results[result_component_i].resize(result_component.buffer_count, hebench::APIBridge::NativeDataBuffer({ 0, 0, 0 }));
            for (std::uint64_t result_sample = 0; result_sample < result_component.buffer_count; ++result_sample)
            {
                assert(offset_p < raw_result_buffer.size());
                raw_results[result_component_i][result_sample].p    = raw_result_buffer.data() + offset_p;
                raw_results[result_component_i][result_sample].size = result_component.p_buffers[result_sample].size;
                raw_results[result_component_i][result_sample].tag  = 0;

                offset_p += raw_results[result_component_i][result_sample].size;
            } // end for
        } // end if
    } // end for

    // DataPack results_pack;
    // results_pack.p_buffers = p_raw_results;
    // results_pack.buffer_count = A_count// B_count;
    // results_pack.param_position = 0; // we are retrieving results in the first position into this data pack

    std::vector<hebench::APIBridge::DataPack> results_packs(raw_results.size());
    for (std::uint64_t result_component_i = 0; result_component_i < results_packs.size(); ++result_component_i)
    {
        results_packs[result_component_i].p_buffers      = raw_results[result_component_i].data();
        results_packs[result_component_i].buffer_count   = raw_results[result_component_i].size();
        results_packs[result_component_i].param_position = result_component_i;
    } // end for

    // PackedData packed_results;
    // packed_results.p_data_packs = &results_pack;
    // packed_results.pack_count = 1; // result shape is [1, 10]

    hebench::APIBridge::PackedData packed_results;
    packed_results.p_data_packs = results_packs.data();
    packed_results.pack_count   = results_packs.size();

    // decode the results

    event_id   = getEventIDNext();
    event_name = "Decoding";

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Decoding...") << std::endl;

    // decode(Handle h_benchmark, h_plain_result, &packed_results);

    timer.start();
    validateRetCode(hebench::APIBridge::decode(handle(), h_plain_results.handle, &packed_results));
    p_timing_event = timer.stop<DefaultTimeInterval>(event_id, 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, event_name);

    // clean up data we no longer need

    // even though it is RAII, it is better to free up space here,
    // just in case these local handles are large
    h_plain_results.destroy();

    std::cout << IOS_MSG_OK << std::endl;

    bool b_valid = true;

    if (run_config.b_validate_results)
    {
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Validation.") << std::endl;

        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Result", false);

        std::size_t mini_reports_cnt        = 3;
        std::size_t report_every_n_elements = num_results / 7 + 1;
        if (report_every_n_elements <= 0)
            report_every_n_elements = 1;
        std::size_t report_every_n_by_3_elements = report_every_n_elements / 4;
        if (report_every_n_by_3_elements <= 0)
            report_every_n_by_3_elements = 1;
        std::size_t progress_report_batch = 0;

        // initialize loop limits
        std::vector<std::size_t> params_count(p_dataset->getParameterCount());
        for (std::uint64_t param_i = 0; param_i < p_dataset->getParameterCount(); ++param_i)
        {
            // ComponentCounter increments starting from component 0,
            // but we want to increment starting from most significant component,
            // so, reverse the sizes:
            params_count[params_count.size() - param_i - 1] = p_dataset->getParameterData(param_i).buffer_count;
        } // end for
        // looping variables
        hebench::Utilities::Math::ComponentCounter param_counter(params_count);
        std::size_t result_i = 0;
        // validate the result of the operation evaluated on all sample combinations per parameter
        do
        {
            assert(result_i < num_results);

            if (result_i == 0 || (result_i + 1) % report_every_n_elements == 0)
            {
                // report operation to show sign of life
                while (mini_reports_cnt++ < 3)
                    std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
                mini_reports_cnt      = 0;
                progress_report_batch = 0;
                std::cout << hebench::Logging::GlobalLogger::log(" " + std::to_string(result_i + 1), false) << std::flush;
            } // end if

            if (mini_reports_cnt < 3
                && progress_report_batch > 0
                && progress_report_batch % report_every_n_by_3_elements == 0)
            {
                // report operation to show sign of life
                ++mini_reports_cnt;
                std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
            } // end if

            // validate output
            std::string s_error_msg;
            std::vector<hebench::APIBridge::NativeDataBuffer *> outputs;
            std::vector<std::uint64_t> data_pack_indices;
            try
            {
                outputs.resize(p_dataset->getResultCount());
                assert(packed_results.pack_count >= outputs.size());
                for (std::uint64_t result_component_i = 0; result_component_i < outputs.size(); ++result_component_i)
                {
                    // make sure the right parameter and result index are being used
                    assert(packed_results.p_data_packs[result_component_i].param_position == result_component_i
                           && packed_results.p_data_packs[result_component_i].buffer_count > result_i);

                    outputs[result_component_i] =
                        &packed_results.p_data_packs[result_component_i].p_buffers[result_i];
                } // end for

                // ComponentCounter increments starting from component 0,
                // but we want to increment starting from most significant component,
                // so, reverse the counter:
                const auto &tmp = param_counter.getCount();
                data_pack_indices.assign(tmp.rbegin(), tmp.rend());
                assert(data_pack_indices.size() == p_dataset->getParameterCount());

                b_valid = validateResult(p_dataset, data_pack_indices.data(),
                                         outputs,
                                         this->getBackendDescription().descriptor.data_type);
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
                   << "Result, " << result_i + 1 << std::endl;
                if (!s_error_msg.empty())
                    ss << s_error_msg << std::endl;
                std::cout << IOS_MSG_FAILED << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;

                // log the parameters, expected result and received result.
                ss << std::endl;
                logResult(ss, p_dataset, data_pack_indices.data(),
                          outputs,
                          this->getBackendDescription().descriptor.data_type);
                out_report.appendFooter(ss.str());
            } // end else

            ++progress_report_batch;
            ++result_i; // next result
        } while (b_valid && !param_counter.inc());

        if (b_valid)
        {
            // report valid op
            if ((num_results) % report_every_n_elements != 0)
            {
                // pretty-print end of result report
                while (mini_reports_cnt++ < 3)
                    std::cout << hebench::Logging::GlobalLogger::log(".", false) << std::flush;
                std::cout << hebench::Logging::GlobalLogger::log(" " + std::to_string(num_results)) << std::endl;
            } // end if
            else
                std::cout << hebench::Logging::GlobalLogger::log("") << std::endl;
            std::cout << IOS_MSG_OK << std::endl;
        } // end if
    } // end if
    else
    {
        out_report.prependFooter("Validation skipped");
        std::cout << IOS_MSG_WARNING << hebench::Logging::GlobalLogger::log("Validation skipped.") << std::endl;
    } // end else

    std::cout << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log("Test Completed.") << std::endl;

    return b_valid;
}

} // namespace TestHarness
} // namespace hebench
