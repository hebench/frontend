
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d

#include <functional>
#include <ostream>
#include <random>
#include <string>

namespace hebench {
namespace Utilities {

constexpr int MaxDecimalDigits = 12;

std::uint64_t copyString(char *dst, std::uint64_t size, const std::string &src);
std::string convertDoubleToStr(double x, int up_to_digits_after_dot = MaxDecimalDigits);
std::string convertDoubleToStrScientific(double x, std::size_t max_width);

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d
