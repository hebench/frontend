
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d

#include <functional>
#include <ostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace hebench {
namespace Utilities {

constexpr int MaxDecimalDigits  = 12;
constexpr const char *BlankTrim = " \t\n\r\f\v";

void ltrim(std::string &s, const char *s_trim = BlankTrim);
void ltrim(std::string_view &s, const char *s_trim = BlankTrim);
void rtrim(std::string &s, const char *s_trim = BlankTrim);
void rtrim(std::string_view &s, const char *s_trim = BlankTrim);
void trim(std::string &s, const char *s_trim = BlankTrim);
void trim(std::string_view &s, const char *s_trim = BlankTrim);

void ToLowerCaseInPlace(std::string &s);
inline std::string ToLowerCase(const std::string_view &s)
{
    std::string retval(s);
    ToLowerCaseInPlace(retval);
    return retval;
}
void ToUpperCaseInPlace(std::string &s);
inline std::string ToUpperCase(const std::string_view &s)
{
    std::string retval(s);
    ToUpperCaseInPlace(retval);
    return retval;
}

std::vector<std::string_view> tokenize(std::string_view s, const std::string_view &delim = " ");

/**
 * @brief Converts a C++ string object into a C-string.
 * @param dst Pointer to output buffer to contain the C-string. Can
 * be null (see details).
 * @param[in] size Size, in bytes of the buffer pointed to by \p dst.
 * @param[in] src C++ string to convert.
 * @return The minimum number of bytes required to convert \p src into
 * \p dst .
 * @details If \p dst is null or \p size is `0`, then, this function does
 * not perform the conversion. This behavior can be used to determine the
 * number of bytes required in a buffer to contain the whole string
 * from \p src from the return value.
 *
 * This function will copy as many bytes as it can from \p src into \p dst .
 * If the size of \p dst buffer is not enough to hold the whole string
 * (plus null terminator), the resulting C-string will be truncated up to
 * the destination size.
 */
std::uint64_t copyString(char *dst, std::uint64_t size, const std::string &src);
/**
 * @brief Converts a floating point value to string with up to a number of
 * digits after decimal point with no trailing zeros.
 * @param[in] x Value to convert.
 * @param[in] up_to_digits_after_dot Maximum number of digits after the decimal point.
 * Lower bound clamped to `0`.
 * @return A string representation of the floating point value with up to
 * the specified number of digits after decimal point with no trailing zeros.
 */
std::string convertDoubleToStr(double x, int up_to_digits_after_dot = MaxDecimalDigits);
/**
 * @brief Converts a double to string in scientific notation, attempting to
 * limit the size of the resulting string to a specified width.
 * @param[in] x Value to convert.
 * @param[in] max_width Maximum width allowed for the resulting string (may
 * be overriden if the value cannot be correctly fitted in the specified width;
 * see details).
 * @return A string representation of the floating point in scientific notation
 * with up to a maximum width size (if correct value fits).
 * @details If the correct scientific notation with 1 decimal place is larger
 * than specified width, then the output string will exceed the width,
 * e.g. 1.234e+02 with a width less than 7 is always 1.2e+02.
 */
std::string convertDoubleToStrScientific(double x, std::size_t max_width);

void writeToFile(const std::string &filename, std::function<void(std::ostream &)> fn,
                 bool b_binary, bool b_append = false);
void writeToFile(const std::string &filename,
                 const char *p_data, std::size_t size,
                 bool b_binary, bool b_append = false);

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d
