
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstring>

#include "tutorial_engine_seal.h"
#include "tutorial_error_seal.h"

// include all benchmarks
#include "tutorial_eltwiseadd_benchmark_seal.h"

//-----------------
// Engine creation
//-----------------

namespace hebench {
namespace cpp {

BaseEngine *createEngine()
{
    // It is a good idea to check here if the API Bridge version is correct for
    // our backend by checking against the constants defined in `hebench/api_bridge/version_seal.h`
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
    addSchemeName(HEBENCH_HE_SCHEME_BFV, "BFV");
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
