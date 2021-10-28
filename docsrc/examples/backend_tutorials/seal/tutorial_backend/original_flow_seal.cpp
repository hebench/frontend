
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/// @cond

//! [tutorial original_flow seal]
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>

#include "seal/seal.h"

class Workload
{
public:
    Workload(std::size_t vector_size);

    std::vector<seal::Plaintext> encodeVector(const std::vector<std::vector<std::int64_t>> &vec);
    std::vector<seal::Ciphertext> encryptVector(const std::vector<seal::Plaintext> &encoded_vec);
    std::vector<seal::Ciphertext> eltwiseadd(const std::vector<seal::Plaintext> &A,
                                             const std::vector<seal::Ciphertext> &B);
    std::vector<seal::Plaintext> decryptResult(const std::vector<seal::Ciphertext> &encrypted_result);
    std::vector<std::vector<int64_t>> decodeResult(const std::vector<seal::Plaintext> &encoded_result);

private:
    class SealBFVContext
    {
    public:
        SealBFVContext(int poly_modulus_degree)
        {
            seal::EncryptionParameters parms{ seal::scheme_type::bfv };
            parms.set_poly_modulus_degree(poly_modulus_degree);
            parms.set_coeff_modulus(seal::CoeffModulus::Create(poly_modulus_degree, { 60, 60 }));
            parms.set_plain_modulus(seal::PlainModulus::Batching(poly_modulus_degree, 20));
            m_p_seal_context.reset(
                new seal::SEALContext(parms, true, seal::sec_level_type::tc128));
            m_p_keygen = std::make_unique<seal::KeyGenerator>(context());

            m_p_keygen->create_public_key(m_public_key);
            m_secret_key = m_p_keygen->secret_key();

            m_p_encryptor     = std::make_unique<seal::Encryptor>(context(), m_public_key);
            m_p_evaluator     = std::make_unique<seal::Evaluator>(context());
            m_p_decryptor     = std::make_unique<seal::Decryptor>(context(), m_secret_key);
            m_p_batch_encoder = std::make_unique<seal::BatchEncoder>(context());
        }

        seal::BatchEncoder &encoder() { return *m_p_batch_encoder; }
        seal::Evaluator &evaluator() { return *m_p_evaluator; }
        seal::Decryptor &decryptor() { return *m_p_decryptor; }
        const seal::Encryptor &encryptor() const { return *m_p_encryptor; }
        const seal::PublicKey &public_key() const { return m_public_key; }
        const seal::SecretKey &secret_key() const { return m_secret_key; }
        seal::SEALContext &context() { return *m_p_seal_context; }

    private:
        std::shared_ptr<seal::SEALContext> m_p_seal_context;
        std::unique_ptr<seal::KeyGenerator> m_p_keygen;
        seal::PublicKey m_public_key;
        seal::SecretKey m_secret_key;
        std::unique_ptr<seal::Encryptor> m_p_encryptor;
        std::unique_ptr<seal::Evaluator> m_p_evaluator;
        std::unique_ptr<seal::Decryptor> m_p_decryptor;
        std::unique_ptr<seal::BatchEncoder> m_p_batch_encoder;
    };

    std::size_t m_vector_size;
    std::shared_ptr<SealBFVContext> m_p_context;

    SealBFVContext &context() { return *m_p_context; }
};

Workload::Workload(std::size_t vector_size)
{
    if (vector_size <= 0)
        throw std::invalid_argument("vector_size");
    m_vector_size = vector_size;
    m_p_context   = std::make_shared<SealBFVContext>(8192);
}

std::vector<seal::Plaintext> Workload::encodeVector(const std::vector<std::vector<std::int64_t>> &vec)
{
    std::vector<seal::Plaintext> retval(vec.size());

    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        assert(vec[i].size() <= context().encoder().slot_count());
        context().encoder().encode(vec[i], retval[i]);
    }
    return retval;
}

std::vector<seal::Ciphertext> Workload::encryptVector(const std::vector<seal::Plaintext> &encoded_vec)
{
    std::vector<seal::Ciphertext> retval(encoded_vec.size());
    for (std::size_t i = 0; i < encoded_vec.size(); i++)
        context().encryptor().encrypt(encoded_vec[i], retval[i]);
    return retval;
}

std::vector<seal::Ciphertext> Workload::eltwiseadd(const std::vector<seal::Plaintext> &A,
                                                   const std::vector<seal::Ciphertext> &B)
{
    std::vector<seal::Ciphertext> retval(A.size() * B.size());

    // This is the main operation function:
    // for an offline test, it must store the result of the operation on every
    // set of input sample in the same order as the input set.
    // See documentation on "Ordering of Results Based on Input Batch Sizes"
    // for details.
    for (std::size_t A_i = 0; A_i < A.size(); ++A_i)
        for (std::size_t B_i = 0; B_i < B.size(); ++B_i)
        {
            seal::Ciphertext &retval_item = retval[A_i * B.size() + B_i];
            context().evaluator().add_plain(B[B_i], A[A_i], retval_item);
        }

    return retval;
}

std::vector<seal::Plaintext> Workload::decryptResult(const std::vector<seal::Ciphertext> &encrypted_result)
{
    std::vector<seal::Plaintext> retval(encrypted_result.size());
    for (std::size_t i = 0; i < encrypted_result.size(); i++)
        context().decryptor().decrypt(encrypted_result[i], retval[i]);
    return retval;
}

std::vector<std::vector<int64_t>> Workload::decodeResult(const std::vector<seal::Plaintext> &encoded_result)
{
    std::vector<std::vector<int64_t>> retval(encoded_result.size());
    for (std::size_t i = 0; i < encoded_result.size(); ++i)
    {
        context().encoder().decode(encoded_result[i], retval[i]);
        retval[i].resize(m_vector_size);
    }
    return retval;
}

//---------------------------------------------------------------------
// main() implementation tests the workflow

int main()
{
    static const std::size_t VectorSize = 400;
    static const std::size_t DimensionA = 2;
    static const std::size_t DimensionB = 5;

    // START data prep
    std::vector<std::vector<std::int64_t>> A(DimensionA, std::vector<std::int64_t>(VectorSize));
    std::vector<std::vector<std::int64_t>> B(DimensionB, std::vector<std::int64_t>(VectorSize));

    // generate and fill vectors with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(-100, 100);

    for (std::size_t i = 0; i < A.size(); ++i)
        for (std::size_t j = 0; j < A[i].size(); ++j)
            A[i][j] = distrib(gen);
    for (std::size_t i = 0; i < B.size(); ++i)
        for (std::size_t j = 0; j < B[i].size(); ++j)
            B[i][j] = distrib(gen);
    // END data prep

    //---------------------------------------------------------------------

    // START Backend
    std::shared_ptr<Workload> p_workload = // initialize
        std::make_shared<Workload>(VectorSize);
    Workload &workload = *p_workload;
    // For illustration purposes only: all functions in the pipeline should
    // be able to be nested with each other in the proper order.
    std::vector<std::vector<std::int64_t>> result =
        workload.decodeResult(
            workload.decryptResult(
                workload.eltwiseadd(workload.encodeVector(A),
                                    workload.encryptVector(workload.encodeVector(B)))));
    p_workload.reset(); // terminate
    // END Backend

    //---------------------------------------------------------------------

    // START Validation

    std::vector<std::vector<std::int64_t>> exp_out(DimensionA * DimensionB, std::vector<std::int64_t>(VectorSize));
    assert(exp_out.size() == result.size());

    // compute ground truth
    std::size_t result_i = 0;
    for (std::size_t A_i = 0; A_i < A.size(); ++A_i)
        for (std::size_t B_i = 0; B_i < B.size(); ++B_i)
        {
            for (std::size_t i = 0; i < VectorSize; ++i)
                exp_out[result_i][i] = A[A_i][i] + B[B_i][i];
            ++result_i;
        }

    if (result == exp_out)
        std::cout << "OK" << std::endl;
    else
        std::cout << "Fail" << std::endl;
    // END Validation

    return 0;
}

//! [tutorial original_flow seal]
/// @endcond
