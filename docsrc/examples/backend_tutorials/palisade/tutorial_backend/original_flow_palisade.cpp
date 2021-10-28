
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/// @cond

//! [tutorial original_flow palisade]
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>

#include "palisade.h"

class Workload
{
public:
    Workload(std::size_t vector_size);

    std::vector<lbcrypto::Plaintext> encodeVector(const std::vector<std::vector<std::int64_t>> &vec);
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> encryptVector(const std::vector<lbcrypto::Plaintext> &encoded_vec);
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> eltwiseadd(const std::vector<lbcrypto::Plaintext> &A,
                                                                     const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &B);
    std::vector<lbcrypto::Plaintext> decryptResult(const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &encrypted_result);
    std::vector<std::vector<int64_t>> decodeResult(const std::vector<lbcrypto::Plaintext> &encoded_result);

private:
    class PalisadeBFVContext
    {
    public:
        PalisadeBFVContext(int poly_modulus_degree)
        {
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

        auto publicKey() const { return m_keys->publicKey; }
        std::size_t getSlotCount() const { return m_slot_count; }
        lbcrypto::CryptoContext<lbcrypto::DCRTPoly> &context() { return *m_p_palisade_context; }
        void decrypt(const lbcrypto::Ciphertext<lbcrypto::DCRTPoly> &cipher, lbcrypto::Plaintext &plain)
        {
            context()->Decrypt(m_keys->secretKey, cipher, &plain);
        }

        lbcrypto::Plaintext decrypt(const lbcrypto::Ciphertext<lbcrypto::DCRTPoly> &cipher)
        {
            lbcrypto::Plaintext retval;
            decrypt(cipher, retval);
            return retval;
        }

    private:
        std::shared_ptr<lbcrypto::CryptoContext<lbcrypto::DCRTPoly>> m_p_palisade_context;
        std::unique_ptr<lbcrypto::LPKeyPair<lbcrypto::DCRTPoly>> m_keys;
        std::size_t m_slot_count;
    };

    std::size_t m_vector_size;
    std::shared_ptr<PalisadeBFVContext> m_p_context;

    PalisadeBFVContext &context() { return *m_p_context; }
};

Workload::Workload(std::size_t vector_size)
{
    if (vector_size <= 0)
        throw std::invalid_argument("vector_size");
    m_vector_size = vector_size;
    m_p_context   = std::make_shared<PalisadeBFVContext>(8192);
}

std::vector<lbcrypto::Plaintext> Workload::encodeVector(const std::vector<std::vector<std::int64_t>> &vec)
{
    std::vector<lbcrypto::Plaintext> retval(vec.size());

    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        assert(vec[i].size() <= context().getSlotCount());
        retval[i] = context().context()->MakePackedPlaintext(vec[i]);
    }
    return retval;
}

std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> Workload::encryptVector(const std::vector<lbcrypto::Plaintext> &encoded_vec)
{
    std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> retval(encoded_vec.size());
    for (std::size_t i = 0; i < encoded_vec.size(); i++)
        retval[i] = context().context()->Encrypt(context().publicKey(), encoded_vec[i]);
    return retval;
}

std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> Workload::eltwiseadd(const std::vector<lbcrypto::Plaintext> &A,
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

std::vector<lbcrypto::Plaintext> Workload::decryptResult(const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &encrypted_result)
{
    std::vector<lbcrypto::Plaintext> retval(encrypted_result.size());
    for (std::size_t i = 0; i < encrypted_result.size(); i++)
        retval[i] = context().decrypt(encrypted_result[i]);
    return retval;
}

std::vector<std::vector<int64_t>> Workload::decodeResult(const std::vector<lbcrypto::Plaintext> &encoded_result)
{
    std::vector<std::vector<int64_t>> retval(encoded_result.size());
    for (std::size_t i = 0; i < encoded_result.size(); ++i)
    {
        retval[i] = encoded_result[i]->GetPackedValue();
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

//! [tutorial original_flow palisade]
/// @endcond
