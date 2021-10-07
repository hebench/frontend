
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "tutorial_eltwiseadd_benchmark.h"
#include "tutorial_engine.h"
#include "tutorial_error.h"

//----------------------------------------------
// class TutorialEltwiseAddBenchmarkDescription
//----------------------------------------------

//! [b_desc constructor]
TutorialEltwiseAddBenchmarkDescription::TutorialEltwiseAddBenchmarkDescription()
{
    //! [b_desc constructor init]
    // initialize the descriptor for this benchmark
    std::memset(&m_descriptor, 0, sizeof(hebench::APIBridge::BenchmarkDescriptor));
    //! [b_desc constructor init]

    //! [b_desc constructor workload]
    m_descriptor.workload = hebench::APIBridge::Workload::EltwiseAdd;
    //! [b_desc constructor workload]

    //! [b_desc constructor data_type]
    m_descriptor.data_type = hebench::APIBridge::DataType::Float64;
    //! [b_desc constructor data_type]

    //! [b_desc constructor category]
    m_descriptor.category = hebench::APIBridge::Category::Offline;
    //! [b_desc constructor category]

    //! [b_desc constructor category_params]
    m_descriptor.cat_params.offline.data_count[0] = 2; // 2 data samples for op parameter 0
    m_descriptor.cat_params.offline.data_count[1] = 5; // 5 data samples for op parameter 1
    //! [b_desc constructor category_params]

    //! [b_desc constructor op_params_mask]
    m_descriptor.cipher_param_mask = 1 << 1;
    //! [b_desc constructor op_params_mask]

    //
    //! [b_desc constructor scheme]
    m_descriptor.scheme = HEBENCH_HE_SCHEME_CKKS;
    //! [b_desc constructor scheme]

    //! [b_desc constructor security]
    m_descriptor.security = TUTORIAL_HE_SECURITY_128;
    //! [b_desc constructor security]

    //! [b_desc constructor other]
    m_descriptor.other = 0; // no extras needed for our purpose:
        // Other back-ends can use this field to differentiate between
        // benchmarks for which internal parameters, not specified by
        // other fields of this structure, differ.
    //! [b_desc constructor other]

    //! [b_desc constructor workload_params]
    // specify default arguments for this workload flexible parameters:
    hebench::cpp::WorkloadParams::EltwiseAdd default_workload_params;
    default_workload_params.n = 1000;
    this->addDefaultParameters(default_workload_params);
    //! [b_desc constructor workload_params]
}
//! [b_desc constructor]

TutorialEltwiseAddBenchmarkDescription::~TutorialEltwiseAddBenchmarkDescription()
{
    // nothing needed in this example
}

//! [b_desc print]
std::string TutorialEltwiseAddBenchmarkDescription::getBenchmarkDescription(const hebench::APIBridge::WorkloadParams *p_w_params) const
{
    std::stringstream ss;
    ss << BenchmarkDescription::getBenchmarkDescription(p_w_params);
    ss << ", Encryption parameters" << std::endl
       << ", , HE Library, Microsoft SEAL 3.5" << std::endl
       << ", , Poly modulus degree, " << PolyModulusDegree << std::endl
       << ", , Coefficient Moduli, 60, ";
    for (std::uint64_t i = 0; i < NumCoefficientModuli - 1; ++i)
        ss << ScaleExponent << ", ";
    ss << "60" << std::endl
       << ", , Scale, 2^" << ScaleExponent;
    if (p_w_params)
    {
        hebench::cpp::WorkloadParams::EltwiseAdd w_params(*p_w_params);
        ss << std::endl
           << ", Workload parameters" << std::endl
           << ", , n, " << w_params.n;
    } // end if
    return ss.str();
}
//! [b_desc print]

hebench::cpp::BaseBenchmark *TutorialEltwiseAddBenchmarkDescription::createBenchmark(hebench::cpp::BaseEngine &engine,
                                                                                     const hebench::APIBridge::WorkloadParams *p_params)
{
    if (!p_params)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid empty workload parameters. This workload requires flexible parameters."),
                                         HEBENCH_ECODE_CRITICAL_ERROR);

    TutorialEngine &ex_engine = dynamic_cast<TutorialEngine &>(engine);
    return new TutorialEltwiseAddBenchmark(ex_engine, m_descriptor, *p_params);
}

void TutorialEltwiseAddBenchmarkDescription::destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench)
{
    // make sure we are destroying a benchmark object we created
    if (p_bench)
    {
        TutorialEltwiseAddBenchmark *p_tmp = dynamic_cast<TutorialEltwiseAddBenchmark *>(p_bench);
        delete p_tmp;
    } // end if
}

//-----------------------------------
// class TutorialEltwiseAddBenchmark
//-----------------------------------

//! [benchmark constructor]
TutorialEltwiseAddBenchmark::TutorialEltwiseAddBenchmark(TutorialEngine &engine,
                                                         const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                         const hebench::APIBridge::WorkloadParams &bench_params) :
    hebench::cpp::BaseBenchmark(engine, bench_desc, bench_params)
{
    // validate workload parameters:
    // these must be checked because users can use benchmark configuration files to pass
    // different parameters

    // number of workload parameters (1 for eltwise add: n)
    if (bench_params.count < TutorialEltwiseAddBenchmarkDescription::NumWorkloadParams)
        // Always throw hebench::cpp::HEBenchError from C++ wrapper to report errors.
        // C++ wrapper will understand this error type and inform Test Harness accordingly.
        // Throwing other exceptions is valid, but they result in Test Harness receiving
        // HEBENCH_ECODE_CRITICAL_ERROR from the back-end.
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters. This workload requires "
                                                            + std::to_string(TutorialEltwiseAddBenchmarkDescription::NumWorkloadParams)
                                                            + "parameters."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // check values of the workload parameters and make sure they are supported by benchmark:
    hebench::cpp::WorkloadParams::EltwiseAdd w_params(bench_params);
    if (w_params.n <= 0
        || w_params.n - 1 > TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree / 2)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters. This workload only supports vectors of size up to "
                                                            + (TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree / 2)),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // Do any extra workload-parameter-based initialization here, if needed.

    // initialize SEAL CKKS
    engine.initCKKS(TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree,
                    TutorialEltwiseAddBenchmarkDescription::NumCoefficientModuli,
                    TutorialEltwiseAddBenchmarkDescription::ScaleExponent);
}
//! [benchmark constructor]

TutorialEltwiseAddBenchmark::~TutorialEltwiseAddBenchmark()
{
    // nothing needed in this example
}

//! [benchmark encode]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::encode(const hebench::APIBridge::PackedData *p_parameters)
{
    // To be compatible with decode and the method signature, this encode bundles
    // multiple data packs inside a single handle. Since in this example we are
    // doing 1 plaintext parameter and 1 ciphertext parameter, each bundle will
    // contain a single data pack.

    assert(p_parameters && p_parameters->pack_count > 0 && p_parameters->p_data_packs);

    TutorialEngine &engine = reinterpret_cast<TutorialEngine &>(this->getEngine());

    //! [benchmark encode allocation]
    // bundle multiple data packs together, even though we will have only 1 per call
    std::vector<InternalParams> params(p_parameters->pack_count);

    for (std::size_t datapack_i = 0; datapack_i < p_parameters->pack_count; ++datapack_i)
    {
        const hebench::APIBridge::DataPack &datapack = p_parameters->p_data_packs[datapack_i];
        assert(datapack.buffer_count > 0 && datapack.p_buffers);

        params[datapack_i].samples.resize(datapack.buffer_count);
        params[datapack_i].param_position = datapack.param_position;
        params[datapack_i].tag            = InternalParams::tagPlaintext;
    }
    //! [benchmark encode allocation]

    //! [benchmark encode encoding]
    for (std::size_t datapack_i = 0; datapack_i < p_parameters->pack_count; ++datapack_i)
    {
        const hebench::APIBridge::DataPack &datapack = p_parameters->p_data_packs[datapack_i];
        for (std::uint64_t sample_i = 0; sample_i < datapack.buffer_count; ++sample_i)
        {
            const hebench::APIBridge::NativeDataBuffer &sample_buffer =
                datapack.p_buffers[sample_i];
            assert(sample_buffer.p && sample_buffer.size / sizeof(double) > 0);
            // read from raw data: the data type is Float64 as specified in this benchmark description
            gsl::span<const double> clear_text =
                gsl::span<const double>(reinterpret_cast<const double *>(sample_buffer.p),
                                        sample_buffer.size / sizeof(double));

            seal::Plaintext encoded;
            engine.ckksEncoder().encode(clear_text, engine.scale(), encoded);
            // store the encoded plaintext in the parameter samples
            params[datapack_i].samples[sample_i] = std::make_shared<seal::Plaintext>(std::move(encoded));
        }
    }
    //! [benchmark encode encoding]

    //! [benchmark encode return]
    // wrap our internal object into a handle to cross the boundary of the API Bridge
    return engine.template createHandle<decltype(params)>(sizeof(seal::Plaintext) * params.size(), // size (arbitrary for our usage if we need to)
                                                          InternalParams::tagPlaintext, // extra tags
                                                          std::move(params)); // constructor parameters
    //! [benchmark encode return]
}
//! [benchmark encode]

//! [benchmark decode]
void TutorialEltwiseAddBenchmark::decode(hebench::APIBridge::Handle h_encoded_data, hebench::APIBridge::PackedData *p_native)
{
    // This decode is able to decode multiple data packs bundled in a single handle

    assert(p_native && p_native->p_data_packs && p_native->pack_count > 0);

    TutorialEngine &engine = reinterpret_cast<TutorialEngine &>(this->getEngine());
    // This method should handle decoding of data encoded using encode(), due to
    // specification stating that encode() and decode() are inverses; as well as
    // handle data decrypted from operation() results.

    // retrieve our internal format object from the handle
    const std::vector<InternalParams> &encoded =
        engine.template retrieveFromHandle<std::vector<InternalParams>>(h_encoded_data);

    // according to specification, we must decode as much data as possible, where
    // any excess encoded data that won't fit into the pre-allocated native buffer
    // shall be ignored

    std::uint64_t min_datapack_count = std::min(p_native->pack_count, encoded.size());
    for (std::size_t datapack_i = 0; datapack_i < min_datapack_count; ++datapack_i)
    {
        // decode the next data pack
        hebench::APIBridge::DataPack *p_native_datapack = &p_native->p_data_packs[datapack_i];

        // find the encoded data pack corresponding to the requested clear text data pack
        const InternalParams *p_encoded_datapack = nullptr;
        for (std::size_t encoded_i = 0; !p_encoded_datapack && encoded_i < encoded.size(); ++encoded_i)
            if (encoded[encoded_i].param_position == p_native_datapack->param_position)
                p_encoded_datapack = &encoded[encoded_i];

        if ((p_encoded_datapack->tag & InternalParams::tagPlaintext) != InternalParams::tagPlaintext)
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected for handle 'h_encoded_data'."),
                                             HEBENCH_ECODE_INVALID_ARGS);

        if (p_native_datapack && p_native_datapack->buffer_count > 0)
        {
            std::uint64_t min_sample_count = std::min(p_native_datapack->buffer_count, encoded[datapack_i].samples.size());
            for (std::uint64_t sample_i = 0; sample_i < min_sample_count; ++sample_i)
            {
                // alias the samples
                hebench::APIBridge::NativeDataBuffer &native_sample = p_native_datapack->p_buffers[sample_i];
                seal::Plaintext &encoded_sample                     = *reinterpret_cast<seal::Plaintext *>(p_encoded_datapack->samples[sample_i].get());

                // decode as much as possible
                std::vector<double> decoded;
                engine.ckksEncoder().decode(encoded_sample, decoded);
                std::copy_n(decoded.begin(),
                            std::min(decoded.size(), native_sample.size / sizeof(double)),
                            reinterpret_cast<double *>(native_sample.p));
            }
        }
    }
}
//! [benchmark decode]

//! [benchmark encrypt]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::encrypt(hebench::APIBridge::Handle h_encoded_parameters)
{
    TutorialEngine &engine = reinterpret_cast<TutorialEngine &>(this->getEngine());

    //! [benchmark encrypt input_handle]
    if ((h_encoded_parameters.tag & InternalParams::tagPlaintext) != InternalParams::tagPlaintext)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected for handle 'h_encoded_parameters'."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    const std::vector<InternalParams> &encoded_parameters =
        engine.template retrieveFromHandle<std::vector<InternalParams>>(h_encoded_parameters);
    //! [benchmark encrypt input_handle]

    std::vector<InternalParams> encrypted_parameters(encoded_parameters.size());

    for (std::size_t datapack_i = 0; datapack_i < encoded_parameters.size(); ++datapack_i)
    {
        if ((encoded_parameters[datapack_i].tag & InternalParams::tagPlaintext) != InternalParams::tagPlaintext)
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected in data pack."),
                                             HEBENCH_ECODE_INVALID_ARGS);

        // encrypt samples into our internal representation

        encrypted_parameters[datapack_i].param_position = encoded_parameters[datapack_i].param_position;
        encrypted_parameters[datapack_i].tag            = InternalParams::tagCiphertext;
        encrypted_parameters[datapack_i].samples.resize(encoded_parameters[datapack_i].samples.size());
        for (size_t sample_i = 0; sample_i < encoded_parameters[datapack_i].samples.size(); sample_i++)
        {
            const seal::Plaintext &encoded_sample =
                *reinterpret_cast<const seal::Plaintext *>(encoded_parameters[datapack_i].samples[sample_i].get());
            seal::Ciphertext encrypted_sample;
            engine.encryptor().encrypt(encoded_sample, encrypted_sample);
            encrypted_parameters[datapack_i].samples[sample_i] =
                std::make_shared<seal::Ciphertext>(std::move(encrypted_sample));
        }
    }

    // wrap our internal object into a handle to cross the boundary of the API Bridge
    return engine.template createHandle<decltype(encrypted_parameters)>(encrypted_parameters.size(),
                                                                        InternalParams::tagCiphertext,
                                                                        std::move(encrypted_parameters));
}
//! [benchmark encrypt]

//! [benchmark decrypt]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::decrypt(hebench::APIBridge::Handle h_encrypted_data)
{
    TutorialEngine &engine = reinterpret_cast<TutorialEngine &>(this->getEngine());

    if ((h_encrypted_data.tag & InternalParams::tagCiphertext) != InternalParams::tagCiphertext)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected for handle 'h_encrypted_data'."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    const std::vector<InternalParams> &encrypted_data =
        engine.template retrieveFromHandle<std::vector<InternalParams>>(h_encrypted_data);

    std::vector<InternalParams> plaintext_data(encrypted_data.size());

    for (std::size_t datapack_i = 0; datapack_i < encrypted_data.size(); ++datapack_i)
    {
        if ((encrypted_data[datapack_i].tag & InternalParams::tagCiphertext) != InternalParams::tagCiphertext)
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected in data pack."),
                                             HEBENCH_ECODE_INVALID_ARGS);

        // decrypt samples into our internal representation

        plaintext_data[datapack_i].param_position = encrypted_data[datapack_i].param_position;
        plaintext_data[datapack_i].tag            = InternalParams::tagPlaintext;
        plaintext_data[datapack_i].samples.resize(encrypted_data[datapack_i].samples.size());
        for (size_t sample_i = 0; sample_i < encrypted_data[datapack_i].samples.size(); sample_i++)
        {
            const seal::Ciphertext &encrypted_sample =
                *reinterpret_cast<const seal::Ciphertext *>(encrypted_data[datapack_i].samples[sample_i].get());
            seal::Plaintext decrypted_sample;
            engine.decryptor().decrypt(encrypted_sample, decrypted_sample);
            plaintext_data[datapack_i].samples[sample_i] =
                std::make_shared<seal::Plaintext>(std::move(decrypted_sample));
        }
    }

    // wrap our internal object into a handle to cross the boundary of the API Bridge
    return engine.template createHandle<decltype(plaintext_data)>(plaintext_data.size(),
                                                                  InternalParams::tagPlaintext,
                                                                  std::move(plaintext_data));
}
//! [benchmark decrypt]

//! [benchmark load]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::load(const hebench::APIBridge::Handle *p_local_data, uint64_t count)
{
    // allocate data for output
    std::vector<InternalParams> loaded_data;

    // bundle copies of the parameters in a single handle
    for (std::size_t handle_i = 0; handle_i < count; ++handle_i)
    {
        const hebench::APIBridge::Handle &handle = p_local_data[handle_i];
        const std::vector<InternalParams> &params =
            this->getEngine().template retrieveFromHandle<std::vector<InternalParams>>(handle);
        // the copy is shallow, but shared_ptr in InternalParams
        // ensures correct destruction using reference counting
        loaded_data.insert(loaded_data.end(), params.begin(), params.end());
    }

    return this->getEngine().template createHandle<decltype(loaded_data)>(loaded_data.size(),
                                                                          InternalParams::tagCiphertext | InternalParams::tagPlaintext,
                                                                          std::move(loaded_data));
}
//! [benchmark load]

//! [benchmark store]
void TutorialEltwiseAddBenchmark::store(hebench::APIBridge::Handle h_remote_data,
                                        hebench::APIBridge::Handle *p_local_data, std::uint64_t count)
{
    if (count > 0 && !p_local_data)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid null array of handles: \"p_local_data\""),
                                         HEBENCH_ECODE_INVALID_ARGS);

    if (count > 0)
    {
        std::vector<InternalParams> plain_data;
        std::vector<InternalParams> encrypted_data;
        const std::vector<InternalParams> &remote_data =
            this->getEngine().template retrieveFromHandle<std::vector<InternalParams>>(h_remote_data);

        // since remote and host are the same for this example, we just need to return a copy
        // of the remote as local data.
        for (const auto &internal_params : remote_data)
        {
            if ((internal_params.tag & InternalParams::tagCiphertext) == InternalParams::tagCiphertext)
                encrypted_data.push_back(internal_params);
            else if ((internal_params.tag & InternalParams::tagPlaintext) == InternalParams::tagPlaintext)
                plain_data.push_back(internal_params);
            else
            {
                throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Unknown tag detected in data pack"),
                                                 HEBENCH_ECODE_INVALID_ARGS);
            }
        }
        if (!encrypted_data.empty())
            // store encrypted data in first handle, as per docs
            p_local_data[0] = this->getEngine().template createHandle<decltype(encrypted_data)>(encrypted_data.size(),
                                                                                                InternalParams::tagCiphertext,
                                                                                                std::move(encrypted_data));
        if (!plain_data.empty())
        {
            // store plain data in next available handle as per docs
            hebench::APIBridge::Handle *p_h_plain = nullptr;
            if (encrypted_data.empty())
                p_h_plain = &p_local_data[0];
            else if (count > 1)
                p_h_plain = &p_local_data[1];

            if (p_h_plain)
                *p_h_plain = this->getEngine().template createHandle<decltype(plain_data)>(plain_data.size(),
                                                                                           InternalParams::tagPlaintext,
                                                                                           std::move(plain_data));
        }
    }

    // pad with zeros any remaining local handles as per specifications
    for (std::uint64_t i = 2; i < count; ++i)
        std::memset(p_local_data + i, 0, sizeof(hebench::APIBridge::Handle));
}
//! [benchmark store]

//! [benchmark operate]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::operate(hebench::APIBridge::Handle h_remote_packed,
                                                                const hebench::APIBridge::ParameterIndexer *p_param_indexers)
{
    TutorialEngine &engine = reinterpret_cast<TutorialEngine &>(this->getEngine());

    //! [benchmark operate load_input]
    if ((h_remote_packed.tag & (InternalParams::tagCiphertext | InternalParams::tagPlaintext)) != (InternalParams::tagCiphertext | InternalParams::tagPlaintext))
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid tag detected for handle 'h_remote_packed'."),
                                         HEBENCH_ECODE_INVALID_ARGS);
    const std::vector<InternalParams> &loaded_data =
        engine.template retrieveFromHandle<std::vector<InternalParams>>(h_remote_packed);

    assert(loaded_data.size() == ParametersCount);
    //! [benchmark operate load_input]

    //! [benchmark operate extract_params]
    // retrieve the plaintext parameter and the ciphertext parameter
    const std::vector<std::shared_ptr<void>> *p_params[ParametersCount] = { nullptr, nullptr };

    for (std::size_t i = 0; i < loaded_data.size(); ++i)
    {
        if (loaded_data[i].param_position == 0
            && (loaded_data[i].tag & InternalParams::tagPlaintext) == InternalParams::tagPlaintext)
            p_params[0] = &loaded_data[i].samples; // param 0 is plain text
        else if (loaded_data[i].param_position == 1
                 && (loaded_data[i].tag & InternalParams::tagCiphertext) == InternalParams::tagCiphertext)
            p_params[1] = &loaded_data[i].samples; // param 1 is ciphertext
    }

    // validate extracted parameters
    for (std::size_t i = 0; i < ParametersCount; ++i)
    {
        if (!p_params[i])
        {
            std::stringstream ss;
            ss << "Unable to find operation parameter " << i << " loaded.";
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()),
                                             HEBENCH_ECODE_INVALID_ARGS);
        }
    }
    //! [benchmark operate extract_params]

    //! [benchmark operate validate_indexers]
    std::uint64_t results_count = 1;
    for (std::size_t param_i = 0; param_i < ParametersCount; ++param_i)
    {
        if (p_param_indexers[param_i].value_index >= p_params[param_i]->size())
        {
            std::stringstream ss;
            ss << "Invalid parameter indexer for operation parameter " << param_i << ". Expected index in range [0, "
               << p_params[param_i]->size() << "), but " << p_param_indexers[param_i].value_index << " received.";
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()),
                                             HEBENCH_ECODE_INVALID_ARGS);
        }
        else if (p_param_indexers[param_i].value_index + p_param_indexers[param_i].batch_size > p_params[param_i]->size())
        {
            std::stringstream ss;
            ss << "Invalid parameter indexer for operation parameter " << param_i << ". Expected batch size in range [1, "
               << p_params[param_i]->size() - p_param_indexers[param_i].value_index << "], but " << p_param_indexers[param_i].batch_size << " received.";
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()),
                                             HEBENCH_ECODE_INVALID_ARGS);
        }
        results_count *= p_param_indexers[param_i].batch_size; // count the number of results expected
    }
    //! [benchmark operate validate_indexers]

    //! [benchmark operate result allocate]
    // allocate space for results
    std::vector<InternalParams> results(ResultComponentsCount);
    results.front().samples.resize(results_count); // ResultComponentsCount == 1 for this workload
    results.front().param_position = 0; // result component
    results.front().tag            = InternalParams::tagCiphertext;
    //! [benchmark operate result allocate]

    //! [benchmark operate operation]
    // perform the actual operation
    // keep in mind the result ordering (for offline test, most significant parameter moves faster)
    std::size_t result_i = 0;
    for (std::uint64_t p0_sample_i = p_param_indexers[0].value_index;
         p0_sample_i < p_param_indexers[0].value_index + p_param_indexers[0].batch_size;
         ++p0_sample_i)
    {
        for (std::uint64_t p1_sample_i = p_param_indexers[1].value_index;
             p1_sample_i < p_param_indexers[1].value_index + p_param_indexers[1].batch_size;
             ++p1_sample_i)
        {
            const seal::Plaintext &p0 =
                *reinterpret_cast<const seal::Plaintext *>(p_params[0]->at(p0_sample_i).get());
            const seal::Ciphertext &p1 =
                *reinterpret_cast<const seal::Ciphertext *>(p_params[1]->at(p1_sample_i).get());
            seal::Ciphertext result;
            engine.evaluator().add_plain(p1, p0, result);

            // store result in our internal representation
            results.front().samples[result_i] = std::make_shared<seal::Ciphertext>(std::move(result));
            ++result_i;
        }
    }
    //! [benchmark operate operation]

    // send our internal result across the boundary of the API Bridge as a handle
    return engine.template createHandle<decltype(results)>(sizeof(seal::Ciphertext) * results_count,
                                                           InternalParams::tagCiphertext,
                                                           std::move(results));
}
//! [benchmark operate]
