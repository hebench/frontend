
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "tutorial_engine.h"
#include "tutorial_error.h"
#include <cstring>

// include all benchmarks
#include "tutorial_eltwiseadd_benchmark.h"

//-----------------
// Engine creation
//-----------------

namespace hebench {
namespace cpp {

BaseEngine *createEngine()
{
    // It is a good idea to check here if the API Bridge version is correct for
    // our backend by checking against the constants defined in `hebench/api_bridge/version.h`
    // HEBENCH_API_VERSION_*
    // For simplicity purposes, no check is performed in this tutorial.

    return TutorialEngine::create();
}

void destroyEngine(BaseEngine *p)
{
    TutorialEngine *_p = dynamic_cast<TutorialEngine *>(p);
    TutorialEngine::destroy(_p);
}

} // namespace cpp
} // namespace hebench

//---------------------
// class ExampleEngine
//---------------------

TutorialEngine *TutorialEngine::create()
{
    TutorialEngine *p_retval = new TutorialEngine();
    p_retval->init();
    return p_retval;
}

void TutorialEngine::destroy(TutorialEngine *p)
{
    if (p)
        delete p;
}

TutorialEngine::TutorialEngine()
{
}

TutorialEngine::~TutorialEngine()
{
}

void TutorialEngine::initCKKS(std::size_t poly_modulus_degree, std::size_t num_coeff_moduli, int scale_exponent)
{
    if (num_coeff_moduli < 2)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid CKKS initialization parameters. There must be, at least, 2 coefficient moduli: \"num_coeff_moduli\"."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    std::vector<int> coeff_modulus = { 60 };
    double scale                   = pow(2.0, scale_exponent);
    for (std::size_t i = 0; i < num_coeff_moduli - 1; ++i)
        coeff_modulus.push_back(scale_exponent);
    coeff_modulus.push_back(60);
    // coeff_modulus = { 60, 40, 40, ..., 60 }
    // coeff_modulus.size() == num_coeff_moduli + 1

    m_scheme = seal::scheme_type::CKKS;
    seal::EncryptionParameters parameters(seal::scheme_type::CKKS);
    parameters.set_poly_modulus_degree(poly_modulus_degree);
    parameters.set_coeff_modulus(seal::CoeffModulus::Create(poly_modulus_degree, coeff_modulus));
    m_context = seal::SEALContext::Create(parameters);
    m_scale   = scale;

    m_keygen       = std::unique_ptr<seal::KeyGenerator>(new seal::KeyGenerator(m_context));
    m_public_key   = std::unique_ptr<seal::PublicKey>(new seal::PublicKey(m_keygen->public_key()));
    m_secret_key   = std::unique_ptr<seal::SecretKey>(new seal::SecretKey(m_keygen->secret_key()));
    m_relin_keys   = std::unique_ptr<seal::RelinKeys>(new seal::RelinKeys(m_keygen->relin_keys_local()));
    m_encryptor    = std::unique_ptr<seal::Encryptor>(new seal::Encryptor(m_context, *m_public_key));
    m_evaluator    = std::unique_ptr<seal::Evaluator>(new seal::Evaluator(m_context));
    m_decryptor    = std::unique_ptr<seal::Decryptor>(new seal::Decryptor(m_context, *m_secret_key));
    m_ckks_encoder = std::unique_ptr<seal::CKKSEncoder>(new seal::CKKSEncoder(m_context));
    m_galois_keys  = std::unique_ptr<seal::GaloisKeys>(new seal::GaloisKeys(m_keygen->galois_keys_local()));
}

//! [engine init]
void TutorialEngine::init()
{
    //! [engine init error codes]
    // add any new error codes

    addErrorCode(TUTORIAL_ECODE_SEAL_ERROR, "SEAL error.");
    //! [engine init error codes]

    //! [engine init schemes]
    // add supported schemes

    //addSchemeName(HEBENCH_HE_SCHEME_PLAIN, "Plain");
    addSchemeName(HEBENCH_HE_SCHEME_CKKS, "CKKS");
    //! [engine init schemes]

    //! [engine init security]
    // add supported security

    //addSecurityName(HEBENCH_HE_SECURITY_NONE, "None");
    addSecurityName(TUTORIAL_HE_SECURITY_128, "128 bits");
    //! [engine init security]

    //! [engine init benchmarks]
    // add the all benchmark descriptors
    addBenchmarkDescription(std::make_shared<TutorialEltwiseAddBenchmarkDescription>());
    //! [engine init benchmarks]
}
//! [engine init]
