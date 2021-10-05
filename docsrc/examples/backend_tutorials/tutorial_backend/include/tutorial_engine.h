
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "hebench/api_bridge/cpp/hebench.hpp"
#include <seal/seal.h>

#define HEBENCH_HE_SCHEME_PLAIN  0
#define HEBENCH_HE_SECURITY_NONE 0
#define TUTORIAL_HE_SECURITY_128 1

class TutorialEngine : public hebench::cpp::BaseEngine
{
public:
    HEBERROR_DECLARE_CLASS_NAME(ExampleEngine)

public:
    static TutorialEngine *create();
    static void destroy(TutorialEngine *p);

    ~TutorialEngine() override;

    void initCKKS(std::size_t poly_modulus_degree, std::size_t num_coeff_moduli, int scale_exponent);

    seal::SEALContext &context() { return *m_context; }
    seal::KeyGenerator &keygen() { return *m_keygen; }
    seal::PublicKey &publicKey() { return *m_public_key; }
    seal::SecretKey &secretKey() { return *m_secret_key; }
    seal::RelinKeys &relin_keys() { return *m_relin_keys; }
    seal::Encryptor &encryptor() { return *m_encryptor; }
    seal::Evaluator &evaluator() { return *m_evaluator; }
    seal::Decryptor &decryptor() { return *m_decryptor; }
    seal::CKKSEncoder &ckksEncoder() { return *m_ckks_encoder; }
    seal::GaloisKeys &galoisKeys() { return *m_galois_keys; }
    double scale() const { return m_scale; }

protected:
    TutorialEngine();

    void init() override;

private:
    std::shared_ptr<seal::SEALContext> m_context;
    std::unique_ptr<seal::KeyGenerator> m_keygen;
    std::unique_ptr<seal::PublicKey> m_public_key;
    std::unique_ptr<seal::SecretKey> m_secret_key;
    std::unique_ptr<seal::RelinKeys> m_relin_keys;
    std::unique_ptr<seal::Encryptor> m_encryptor;
    std::unique_ptr<seal::Evaluator> m_evaluator;
    std::unique_ptr<seal::Decryptor> m_decryptor;
    std::unique_ptr<seal::CKKSEncoder> m_ckks_encoder;
    std::unique_ptr<seal::GaloisKeys> m_galois_keys;
    seal::scheme_type m_scheme;
    double m_scale;
};
