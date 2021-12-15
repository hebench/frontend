
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Types_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Types_H_0596d40a3cce4b108a81595c50eb286d

#include <cstdint>
#include <functional>
#include <memory>
#include <ratio>
#include <string>
#include <unordered_map>
#include <vector>

#include "hebench/api_bridge/types.h"

#define IOS_MSG_DONE    "[    DONE ] "
#define IOS_MSG_WARNING "[ WARNING ] "
#define IOS_MSG_ERROR   "[   ERROR ] "
#define IOS_MSG_FAILED  "[    FAIL ] "
#define IOS_MSG_INFO    "[ Info    ] "
#define IOS_MSG_OK      "[      OK ] "

namespace hebench {
namespace TestHarness {

using DefaultTimeInterval = std::micro;

template <typename T>
using unique_ptr_custom_deleter = std::unique_ptr<T, std::function<void(T *)>>;

constexpr const char *FileNameNoExtReport  = "report";
constexpr const char *FileNameNoExtSummary = "summary";

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Types_H_0596d40a3cce4b108a81595c50eb286d
