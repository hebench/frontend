
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

protected:
    TutorialEngine();

    void init() override;
};
