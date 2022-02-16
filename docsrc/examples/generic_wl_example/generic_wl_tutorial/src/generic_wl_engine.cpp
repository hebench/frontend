
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/// @cond

#include "../include/generic_wl_engine.h"
#include <cstring>

// include all benchmarks
#include "../include/generic_wl_benchmark.h"

//-----------------
// Engine creation
//-----------------

namespace hebench {
namespace cpp {

BaseEngine *createEngine()
{
    return ExampleEngine::create();
}

void destroyEngine(BaseEngine *p)
{
    ExampleEngine *_p = dynamic_cast<ExampleEngine *>(p);
    ExampleEngine::destroy(_p);
}

} // namespace cpp
} // namespace hebench

//---------------------
// class ExampleEngine
//---------------------

ExampleEngine *ExampleEngine::create()
{
    ExampleEngine *p_retval = new ExampleEngine();
    p_retval->init();
    return p_retval;
}

void ExampleEngine::destroy(ExampleEngine *p)
{
    if (p)
        delete p;
}

ExampleEngine::ExampleEngine()
{
}

ExampleEngine::~ExampleEngine()
{
}

void ExampleEngine::init()
{
    // add any new error codes: use

    // this->addErrorCode(code, "generic description");

    // add supported schemes

    addSchemeName(HEBENCH_HE_SCHEME_PLAIN, "Plain");

    // add supported security

    addSecurityName(HEBENCH_HE_SECURITY_NONE, "None");

    // add the all benchmark descriptors
    addBenchmarkDescription(std::make_shared<ExampleBenchmarkDescription>(hebench::APIBridge::Category::Latency));
    addBenchmarkDescription(std::make_shared<ExampleBenchmarkDescription>(hebench::APIBridge::Category::Offline));
}

/// @endcond
