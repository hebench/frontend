
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

/**
 * @brief Opens a file for writing and calls the specified function passing the corresponding stream.
 * @param[in] filename Name of file to open.
 * @param[in] fn Function to be called to write to the stream for the open file.
 * @param[in] b_binary If true, file will be open as binary, otherwise, it will be open
 * for text output.
 * @param[in] b_append If true, the file will be open in append mode, otherwise, the file
 * will be overwritten and open as new.
 */
void writeToFile(const std::string &filename, std::function<void(std::ostream &)> fn,
                 bool b_binary, bool b_append = false);
/**
 * @brief Writes the specified data into a file.
 * @param[in] filename Name of file to open.
 * @param[in] p_data Pointer to data to write into the file. Cannot be `null`.
 * @param[in] size Size, in bytes for the data pointed by \p p_data .
 * @param[in] b_binary If true, data will be stored as in in binary mode, otherwise, it will be
 * stored as text.
 * @param[in] b_append If true, the data will be appended to the current contents of the file,
 * otherwise, the file contents will be replaced by the data.
 */
void writeToFile(const std::string &filename,
                 const char *p_data, std::size_t size,
                 bool b_binary, bool b_append = false);

class CSVTokenizer
{
public:
    /**
     * @brief Splits a CSV line in next CSV value and the rest of the line.
     * @param[in] s_line Input CSV line.
     * @param[in] delim Delimiter used to separate values. Defaults to comma "`,`" .
     * @return Pair of `(car, cdr)` for \p s_line .
     * @details Returns `(car, cdr)` for \p s_line , where `car` is next CSV value,
     * and `cdr` is the rest of the csv line starting after the last comma, or
     * empty if no more values.
     */
    static std::pair<std::string_view, std::string_view> findNextValue(const std::string_view &s_line, char delim);

    /**
     * @brief Extracts the values from a CSV in place.
     * @param[in] s_line String view of the line to tokenize.
     * @param[in] delim Delimiter used to separate values. Defaults to comma "`,`" .
     * @return A vector of views inside \p s_line where each view corresponds to a token.
     * @details Tokens preserve surrounding quotations if they exist. Tokens are trimmed
     * left and right from blank spaces.
     *
     * Right-most blank token is ignored. This is, if a CSV line ends with several commas,
     * the last value is ignored. e.g. (1, 2,,,) generates the tokens 1, 2, <empty_string>,
     * and <empty_string>.
     *
     * Quotations cannot be escaped. Form for tokens that have quotations in mid token, or
     * multiple quotations is undefined. Quotations must be surrounding a single token.
     * Two adjacent quotations are read normally into the same token. Clients can use
     * this behavior if they want to process two quotations as a single, escaped quotation.
     *
     * The input \p s_line must be valid for the result to be valid since the result is
     * composed of views inside the input.
     */
    static std::vector<std::string_view> tokenizeLineInPlace(const std::string_view &s_line, char delim = ',');

    /**
     * @brief Extracts the values from a CSV.
     * @param[in] s_line String view of the line to tokenize.
     * @param[in] delim Delimiter used to separate values. Defaults to comma "`,`" .
     * @return A vector of strings where each string corresponds to a token.
     * @details Tokens preserve surrounding quotations if they exist. Tokens are trimmed
     * left and right from blank spaces.
     *
     * Right-most blank token is ignored. This is, if a CSV line ends with several commas,
     * the last value is ignored. e.g. (1, 2,,,) generates the tokens 1, 2, <empty_string>,
     * and <empty_string>.
     *
     * Quotations cannot be escaped. Form for tokens that have quotations in mid token, or
     * multiple quotations is undefined. Quotations must be surrounding a single token.
     * Two adjacent quotations are read normally into the same token. Clients can use
     * this behavior if they want to process two quotations as a single, escaped quotation.
     *
     * Resulting tokens are stand alone. Input \p s_line can be disposed of once this
     * function completes.
     */
    static std::vector<std::string> tokenizeLine(const std::string_view &s_line, char delim = ',');
};

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Common_Utilities_H_0596d40a3cce4b108a81595c50eb286d
