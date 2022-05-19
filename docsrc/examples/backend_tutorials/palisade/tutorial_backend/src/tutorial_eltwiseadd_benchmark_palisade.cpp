
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/// @cond

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "tutorial_eltwiseadd_benchmark_palisade.h"
#include "tutorial_engine_palisade.h"
#include "tutorial_error_palisade.h"

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
    m_descriptor.data_type = hebench::APIBridge::DataType::Int64;
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
    m_descriptor.scheme = HEBENCH_HE_SCHEME_BFV;
    //! [b_desc constructor scheme]

    //! [b_desc constructor security]
    m_descriptor.security = TUTORIAL_HE_SECURITY_128;
    //! [b_desc constructor security]

    //! [b_desc constructor other]
    m_descriptor.other = 0; // no extras needed for our purpose:
        // Other backends can use this field to differentiate between
        // benchmarks for which internal parameters, not specified by
        // other fields of this structure, differ.
    //! [b_desc constructor other]

    //! [b_desc constructor workload_params]
    // specify default arguments for this workload flexible parameters:
    hebench::cpp::WorkloadParams::EltwiseAdd default_workload_params;
    default_workload_params.n() = 400;
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
       << ", , HE Library, PALISADE v1.11.3" << std::endl
       << ", , Poly modulus degree, " << PolyModulusDegree << std::endl
       << ", , Coefficient Moduli, 60, 60";
    if (p_w_params)
    {
        hebench::cpp::WorkloadParams::EltwiseAdd w_params(*p_w_params);
        ss << std::endl
           << ", Workload parameters" << std::endl
           << ", , n, " << w_params.n();
    } // end if
    return ss.str();
}
//! [b_desc print]

//! [b_desc create]
hebench::cpp::BaseBenchmark *TutorialEltwiseAddBenchmarkDescription::createBenchmark(hebench::cpp::BaseEngine &engine,
                                                                                     const hebench::APIBridge::WorkloadParams *p_params)
{
    if (!p_params)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid empty workload parameters. This workload requires flexible parameters."),
                                         HEBENCH_ECODE_CRITICAL_ERROR);

    TutorialEngine &ex_engine = dynamic_cast<TutorialEngine &>(engine);
    return new TutorialEltwiseAddBenchmark(ex_engine, m_descriptor, *p_params);
}
//! [b_desc create]

//! [b_desc destroy]
void TutorialEltwiseAddBenchmarkDescription::destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench)
{
    if (p_bench)
        delete p_bench;
}
//! [b_desc destroy]

//---------------------------------------------
// class TutorialEltwiseAddBenchmark::Workload
//---------------------------------------------

//! [benchmark src workflow]
TutorialEltwiseAddBenchmark::Workload::PalisadeBFVContext::PalisadeBFVContext(int poly_modulus_degree)
{
    assert(TutorialEltwiseAddBenchmarkDescription::NumCoefficientModuli == 1);
    std::size_t plaintext_modulus     = 65537;
    lbcrypto::SecurityLevel sec_level = lbcrypto::HEStd_128_classic;
    double sigma                      = 3.2;
    std::size_t num_coeff_moduli      = 2;
    std::size_t max_depth             = 2;
    std::size_t coeff_moduli_bits     = 40;

    lbcrypto::CryptoContext<lbcrypto::DCRTPoly> crypto_context =
        lbcrypto::CryptoContextFactory<lbcrypto::DCRTPoly>::genCryptoContextBFVrns(
            plaintext_modulus, sec_level, sigma, 0, num_coeff_moduli,
            0, OPTIMIZED, max_depth, 0, coeff_moduli_bits, poly_modulus_degree);
    crypto_context->Enable(ENCRYPTION);
    crypto_context->Enable(SHE);

    lbcrypto::LPKeyPair<lbcrypto::DCRTPoly> local_key = crypto_context->KeyGen();
    lbcrypto::LPKeyPair<lbcrypto::DCRTPoly> *p_key    = new lbcrypto::LPKeyPair<lbcrypto::DCRTPoly>(local_key.publicKey, local_key.secretKey);
    local_key                                         = lbcrypto::LPKeyPair<lbcrypto::DCRTPoly>();
    m_keys                                            = std::unique_ptr<lbcrypto::LPKeyPair<lbcrypto::DCRTPoly>>(p_key);

    m_p_palisade_context = std::make_shared<lbcrypto::CryptoContext<lbcrypto::DCRTPoly>>(crypto_context);
    m_slot_count         = poly_modulus_degree;
}

TutorialEltwiseAddBenchmark::Workload::Workload(std::size_t vector_size)
{
    if (vector_size <= 0)
        throw std::invalid_argument("vector_size");
    m_vector_size = vector_size;
    m_p_context   = std::make_shared<PalisadeBFVContext>(TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree);
}

std::vector<lbcrypto::Plaintext> TutorialEltwiseAddBenchmark::Workload::encodeVector(const std::vector<std::vector<std::int64_t>> &vec)
{
    std::vector<lbcrypto::Plaintext> retval(vec.size());

    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        assert(vec[i].size() <= context().getSlotCount());
        retval[i] = context().context()->MakePackedPlaintext(vec[i]);
    }
    return retval;
}

std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> TutorialEltwiseAddBenchmark::Workload::encryptVector(const std::vector<lbcrypto::Plaintext> &encoded_vec)
{
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> retval(encoded_vec.size());
    for (std::size_t i = 0; i < encoded_vec.size(); i++)
        retval[i] = context().context()->Encrypt(context().publicKey(), encoded_vec[i]);
    return retval;
}

std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> TutorialEltwiseAddBenchmark::Workload::eltwiseadd(const std::vector<lbcrypto::Plaintext> &A,
                                                                                                        const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &B)
{
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> retval(A.size() * B.size());

    // This is the main operation function:
    // for an offline test, it must store the result of the operation on every
    // set of input sample in the same order as the input set.
    // See documentation on "Ordering of Results Based on Input Batch Sizes"
    // for details.
    for (std::size_t A_i = 0; A_i < A.size(); ++A_i)
        for (std::size_t B_i = 0; B_i < B.size(); ++B_i)
        {
            lbcrypto::Ciphertext<lbcrypto::DCRTPoly> &retval_item = retval[A_i * B.size() + B_i];
            retval_item                                           = context().context()->EvalAdd(B[B_i], A[A_i]);
        }

    return retval;
}

std::vector<lbcrypto::Plaintext> TutorialEltwiseAddBenchmark::Workload::decryptResult(const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &encrypted_result)
{
    std::vector<lbcrypto::Plaintext> retval(encrypted_result.size());
    for (std::size_t i = 0; i < encrypted_result.size(); i++)
        retval[i] = context().decrypt(encrypted_result[i]);
    return retval;
}

std::vector<std::vector<int64_t>> TutorialEltwiseAddBenchmark::Workload::decodeResult(const std::vector<lbcrypto::Plaintext> &encoded_result)
{
    std::vector<std::vector<int64_t>> retval(encoded_result.size());
    for (std::size_t i = 0; i < encoded_result.size(); ++i)
    {
        retval[i] = encoded_result[i]->GetPackedValue();
        retval[i].resize(m_vector_size);
    }
    return retval;
}
//! [benchmark src workflow]

//-----------------------------------
// class TutorialEltwiseAddBenchmark
//-----------------------------------

//! [benchmark constructor]
TutorialEltwiseAddBenchmark::TutorialEltwiseAddBenchmark(hebench::cpp::BaseEngine &engine,
                                                         const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                         const hebench::APIBridge::WorkloadParams &bench_params) :
    hebench::cpp::BaseBenchmark(engine, bench_desc, bench_params)
{
    // validate workload parameters

    // number of workload parameters (1 for eltwise add: n)
    if (bench_params.count < TutorialEltwiseAddBenchmarkDescription::NumWorkloadParams)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters. This workload requires "
                                                            + std::to_string(TutorialEltwiseAddBenchmarkDescription::NumWorkloadParams)
                                                            + "parameters."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // check values of the workload parameters and make sure they are supported by benchmark:
    hebench::cpp::WorkloadParams::EltwiseAdd w_params(bench_params);
    if (w_params.n() <= 0
        || w_params.n() - 1 > TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree / 2)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters. This workload only supports vectors of size up to "
                                                            + std::to_string(TutorialEltwiseAddBenchmarkDescription::PolyModulusDegree / 2)),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // Do any extra workload-parameter-based initialization here, if needed.

    // initialize original Workload (this initializes PALISADE BFV context)
    m_p_workload = std::make_shared<Workload>(w_params.n());
}
//! [benchmark constructor]

//! [benchmark destructor]
TutorialEltwiseAddBenchmark::~TutorialEltwiseAddBenchmark()
{
    m_p_workload.reset(); // added for clarity
}
//! [benchmark destructor]

//! [benchmark encode]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::encode(const hebench::APIBridge::PackedData *p_parameters)
{
    assert(p_parameters && p_parameters->pack_count > 0 && p_parameters->p_data_packs);

    assert(p_parameters->pack_count == 1);

    //! [benchmark encode validation]
    const hebench::APIBridge::DataPack &param_pack = p_parameters->p_data_packs[0];

    if (param_pack.buffer_count != this->getDescriptor().cat_params.offline.data_count[param_pack.param_position])
    {
        std::stringstream ss;
        ss << "Unexpected number of input samples for operation parameter " << param_pack.param_position
           << ". Expected " << this->getDescriptor().cat_params.offline.data_count[param_pack.param_position]
           << ", but " << std::to_string(param_pack.buffer_count) << " received.";
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()));
    }
    //! [benchmark encode validation]

    //! [benchmark encode preparation]
    std::vector<std::vector<std::int64_t>> clear_param(param_pack.buffer_count);

    for (std::size_t sample_i = 0; sample_i < clear_param.size(); ++sample_i)
    {
        const hebench::APIBridge::NativeDataBuffer &native_sample = param_pack.p_buffers[sample_i];
        const std::int64_t *start_pt                              = reinterpret_cast<const std::int64_t *>(native_sample.p);
        const std::int64_t *end_pt                                = start_pt + native_sample.size / sizeof(std::int64_t);
        clear_param[sample_i]                                     = std::vector<std::int64_t>(start_pt, end_pt);
    }
    //! [benchmark encode preparation]

    //! [benchmark encode encoding]
    std::vector<lbcrypto::Plaintext> encoded = m_p_workload->encodeVector(clear_param);
    //! [benchmark encode encoding]
    //! [benchmark encode return]
    InternalParam<lbcrypto::Plaintext> retval;
    retval.samples        = std::move(encoded);
    retval.param_position = param_pack.param_position;
    retval.tag            = InternalParamInfo::tagPlaintext;

    return this->getEngine().template createHandle<decltype(retval)>(
        sizeof(lbcrypto::Plaintext) * retval.samples.size(), // size (arbitrary for our usage if we need to)
        retval.tag, // extra tags
        std::move(retval)); // constructor parameters
    //! [benchmark encode return]
}
//! [benchmark encode]

//! [benchmark decode]
void TutorialEltwiseAddBenchmark::decode(hebench::APIBridge::Handle h_encoded_data,
                                         hebench::APIBridge::PackedData *p_native)
{
    assert(p_native && p_native->p_data_packs && p_native->pack_count > 0);

    //! [benchmark decode input_handle]
    const InternalParam<lbcrypto::Plaintext> &encoded_data =
        this->getEngine().retrieveFromHandle<InternalParam<lbcrypto::Plaintext>>(h_encoded_data, InternalParamInfo::tagPlaintext);
    //! [benchmark decode input_handle]

    //! [benchmark decode decoding]
    std::vector<std::vector<std::int64_t>> clear_result = m_p_workload->decodeResult(encoded_data.samples);
    //! [benchmark decode decoding]

    //! [benchmark decode re_formatting]
    hebench::APIBridge::DataPack &native_datapack = this->findDataPack(*p_native, encoded_data.param_position);

    std::uint64_t min_sample_count = std::min(native_datapack.buffer_count, clear_result.size());
    for (std::uint64_t sample_i = 0; sample_i < min_sample_count; ++sample_i)
    {
        // alias the samples
        hebench::APIBridge::NativeDataBuffer &native_sample = native_datapack.p_buffers[sample_i];
        // copy as much as possible
        const std::vector<std::int64_t> &decoded = clear_result[sample_i];
        std::uint64_t min_size                   = std::min(decoded.size(), native_sample.size / sizeof(std::int64_t));
        std::copy_n(decoded.begin(), min_size,
                    reinterpret_cast<std::int64_t *>(native_sample.p));
    }
    //! [benchmark decode re_formatting]
}
//! [benchmark decode]

//! [benchmark encrypt]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::encrypt(hebench::APIBridge::Handle h_encoded_parameters)
{
    //! [benchmark encrypt input_handle]
    const InternalParam<lbcrypto::Plaintext> &encoded_parameter =
        this->getEngine().template retrieveFromHandle<InternalParam<lbcrypto::Plaintext>>(h_encoded_parameters,
                                                                                          InternalParamInfo::tagPlaintext);
    //! [benchmark encrypt input_handle]

    //! [benchmark encrypt encrypting]
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> encrypted = m_p_workload->encryptVector(encoded_parameter.samples);
    //! [benchmark encrypt encrypting]

    //! [benchmark encrypt return]
    InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> retval;
    retval.samples        = std::move(encrypted);
    retval.param_position = encoded_parameter.param_position;
    retval.tag            = InternalParamInfo::tagCiphertext;

    return this->getEngine().template createHandle<decltype(retval)>(
        sizeof(lbcrypto::Ciphertext<lbcrypto::DCRTPoly>) * retval.samples.size(), // size (arbitrary for our usage if we need to)
        retval.tag, // extra tags
        std::move(retval)); // constructor parameters
    //! [benchmark encrypt return]
}
//! [benchmark encrypt]

//! [benchmark decrypt]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::decrypt(hebench::APIBridge::Handle h_encrypted_data)
{
    //! [benchmark decrypt input_handle]
    const InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &encrypted_data =
        this->getEngine().retrieveFromHandle<InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>>>(h_encrypted_data, InternalParamInfo::tagCiphertext);
    //! [benchmark decrypt input_handle]

    assert(encrypted_data.param_position == 0);

    //! [benchmark decrypt decrypting]
    std::vector<lbcrypto::Plaintext> encoded_data_samples = m_p_workload->decryptResult(encrypted_data.samples);
    //! [benchmark decrypt decrypting]

    //! [benchmark decrypt return]
    InternalParam<lbcrypto::Plaintext> retval;
    retval.samples        = std::move(encoded_data_samples);
    retval.param_position = encrypted_data.param_position;
    retval.tag            = InternalParamInfo::tagPlaintext;

    return this->getEngine().template createHandle<decltype(retval)>(
        sizeof(lbcrypto::Plaintext) * retval.samples.size(), // size (arbitrary for our usage if we need to)
        retval.tag, // extra tags
        std::move(retval)); // move to avoid copy
    //! [benchmark decrypt return]
}
//! [benchmark decrypt]

//! [benchmark load]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::load(const hebench::APIBridge::Handle *p_local_data, uint64_t count)
{
    assert(count == TutorialEltwiseAddBenchmarkDescription::ParametersCount);

    //! [benchmark load op_input_format]
    std::pair<std::vector<lbcrypto::Plaintext>, std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>>> params;
    //! [benchmark load op_input_format]

    //! [benchmark load input_handle]
    // We query for the parameter position, and, once found, we create a copy of the data.
    for (std::size_t handle_i = 0; handle_i < count; ++handle_i)
    {
        const InternalParamInfo &param_info =
            this->getEngine().retrieveFromHandle<InternalParamInfo>(p_local_data[handle_i]);
        assert(param_info.param_position < TutorialEltwiseAddBenchmarkDescription::ParametersCount);

        switch (param_info.param_position)
        {
        case 0:
        {
            if (!params.first.empty())
                throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Duplicated operation parameter detected in input handle."),
                                                 HEBENCH_ECODE_INVALID_ARGS);
            const InternalParam<lbcrypto::Plaintext> &internal_param =
                this->getEngine().retrieveFromHandle<InternalParam<lbcrypto::Plaintext>>(p_local_data[handle_i],
                                                                                         InternalParamInfo::tagPlaintext);
            // create a deep copy of input
            params.first = internal_param.samples;
            break;
        }
        case 1:
        {
            if (!params.second.empty())
                throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Duplicated operation parameter detected in input handle."),
                                                 HEBENCH_ECODE_INVALID_ARGS);
            // create a deep copy of input
            const InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &internal_param =
                this->getEngine().retrieveFromHandle<InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>>>(p_local_data[handle_i],
                                                                                                              InternalParamInfo::tagCiphertext);
            // create a deep copy of input
            params.second = internal_param.samples;
            break;
        }
        } // end switch
    } // end for
    //! [benchmark load input_handle]

    //! [benchmark load return]
    return this->getEngine().template createHandle<decltype(params)>(
        sizeof(params), // size (arbitrary for our usage if we need to)
        InternalParamInfo::tagPlaintext | InternalParamInfo::tagCiphertext, // extra tags
        std::move(params)); // move to avoid extra copy
    //! [benchmark load return]
}
//! [benchmark load]

//! [benchmark store]
void TutorialEltwiseAddBenchmark::store(hebench::APIBridge::Handle h_remote_data,
                                        hebench::APIBridge::Handle *p_local_data, std::uint64_t count)
{
    if (count > 0)
    {
        //! [benchmark store specification]
        std::memset(p_local_data, 0, sizeof(hebench::APIBridge::Handle) * count);
        //! [benchmark store specification]

        //! [benchmark store duplicate]
        p_local_data[0] = this->getEngine().duplicateHandle(h_remote_data,
                                                            InternalParamInfo::tagCiphertext); // validate that we are operating on the correct handle
        //! [benchmark store duplicate]
    }
}
//! [benchmark store]

//! [benchmark operate]
hebench::APIBridge::Handle TutorialEltwiseAddBenchmark::operate(hebench::APIBridge::Handle h_remote_packed,
                                                                const hebench::APIBridge::ParameterIndexer *p_param_indexers)
{
    //! [benchmark operate load_input]
    const std::pair<std::vector<lbcrypto::Plaintext>, std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>>> &params =
        this->getEngine().retrieveFromHandle<std::pair<std::vector<lbcrypto::Plaintext>, std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>>>>(
            h_remote_packed, InternalParamInfo::tagCiphertext | InternalParamInfo::tagPlaintext);

    // Looks familiar?
    const std::vector<lbcrypto::Plaintext> &A                      = params.first;
    const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &B = params.second;
    //! [benchmark operate load_input]

    //! [benchmark operate validate_indexers]
    std::array<std::size_t, TutorialEltwiseAddBenchmarkDescription::ParametersCount> param_size;
    param_size[0]               = A.size();
    param_size[1]               = B.size();
    std::uint64_t results_count = 1;
    for (std::size_t param_i = 0; param_i < TutorialEltwiseAddBenchmarkDescription::ParametersCount; ++param_i)
    {
        if (p_param_indexers[param_i].value_index >= param_size[param_i])
        {
            std::stringstream ss;
            ss << "Invalid parameter indexer for operation parameter " << param_i << ". Expected index in range [0, "
               << param_size[param_i] << "), but " << p_param_indexers[param_i].value_index << " received.";
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()),
                                             HEBENCH_ECODE_INVALID_ARGS);
        }
        else if (p_param_indexers[param_i].value_index + p_param_indexers[param_i].batch_size > param_size[param_i])
        {
            std::stringstream ss;
            ss << "Invalid parameter indexer for operation parameter " << param_i << ". Expected batch size in range [1, "
               << param_size[param_i] - p_param_indexers[param_i].value_index << "], but " << p_param_indexers[param_i].batch_size << " received.";
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS(ss.str()),
                                             HEBENCH_ECODE_INVALID_ARGS);
        }
        results_count *= p_param_indexers[param_i].batch_size; // count the number of results expected
    }
    //! [benchmark operate validate_indexers]

    //! [benchmark operate operation]
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> result = m_p_workload->eltwiseadd(A, B);
    assert(result.size() == results_count);
    //! [benchmark operate operation]

    //! [benchmark operate return]
    // Finally, we wrap the result in our internal representation.
    InternalParam<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> retval;
    retval.samples        = std::move(result);
    retval.param_position = 0; // position of this result component inside the result tuple.
    retval.tag            = InternalParamInfo::tagCiphertext;

    // Hide our representation inside an opaque handle to cross the boundary of the API Bridge.
    // This handle will be passed to method `store()` in the default pipeline.
    return this->getEngine().template createHandle<decltype(retval)>(
        sizeof(lbcrypto::Ciphertext<lbcrypto::DCRTPoly>) * retval.samples.size(), // size (arbitrary for our usage if we need to)
        retval.tag, // extra tags
        std::move(retval)); // move to avoid extra copies
    //! [benchmark operate return]
}
//! [benchmark operate]

/// @endcond
