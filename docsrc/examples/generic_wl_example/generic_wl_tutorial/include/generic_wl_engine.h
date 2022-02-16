
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

/// @cond

#include "hebench/api_bridge/cpp/hebench.hpp"

#define HEBENCH_HE_SCHEME_PLAIN  0
#define HEBENCH_HE_SECURITY_NONE 0

class ExampleEngine : public hebench::cpp::BaseEngine
{
public:
    HEBERROR_DECLARE_CLASS_NAME(ExampleEngine)

public:
    static ExampleEngine *create();
    static void destroy(ExampleEngine *p);

    ~ExampleEngine() override;

protected:
    ExampleEngine();

    void init() override;
};

/// @endcond
