
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../include/hebench_utilities.h"

namespace hebench {
namespace Utilities {

void ltrim(std::string &s, const char *s_trim)
{
    s.erase(0, s.find_first_not_of(s_trim));
}

void ltrim(std::string_view &s, const char *s_trim)
{
    auto pos = s.find_first_not_of(s_trim);
    s.remove_prefix((pos > s.size() ? s.size() : pos));
}

void rtrim(std::string &s, const char *s_trim)
{
    s.erase(s.find_last_not_of(s_trim) + 1);
}

void rtrim(std::string_view &s, const char *s_trim)
{
    auto pos = s.find_last_not_of(s_trim);
    s.remove_suffix(s.size() - (pos > s.size() ? 0 : pos + 1));
}

void trim(std::string &s, const char *s_trim)
{
    s.erase(0, s.find_first_not_of(s_trim));
    s.erase(s.find_last_not_of(s_trim) + 1);
}

void trim(std::string_view &s, const char *s_trim)
{
    ltrim(s, s_trim);
    rtrim(s, s_trim);
}

void ToLowerCaseInPlace(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

void ToUpperCaseInPlace(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
}

std::vector<std::string_view> tokenize(std::string_view s, const std::string_view &delim)
{
    std::vector<std::string_view> retval;

    while (!s.empty())
    {
        auto pos = s.find(delim);
        if (pos == std::string_view::npos)
            pos = s.size();
        retval.emplace_back(s.substr(0, pos));
        s.remove_prefix(pos);
        if (!s.empty())
            s.remove_prefix(delim.size());
    }

    return retval;
}

std::uint64_t copyString(char *dst, std::uint64_t size, const std::string &src)
{
    std::uint64_t retval = src.size() + 1;

    if (dst && size > 0)
    {
        std::uint64_t min_size = std::min(size, retval);
        if (min_size > 1)
            std::copy_n(src.c_str(), min_size - 1, dst);
        dst[min_size - 1] = '\0'; // close string
    } // end if

    return retval;
}

std::string convertDoubleToStr(double x, int up_to_digits_after_dot)
{
    std::string retval;
    std::stringstream ss;
    if (up_to_digits_after_dot < 0)
        up_to_digits_after_dot = 0;
    ss << std::fixed << std::setprecision(up_to_digits_after_dot) << x;
    retval = ss.str();
    retval.erase(retval.find_last_not_of("0") + 1);
    retval.erase(retval.find_last_not_of(".") + 1);
    return retval;
}

std::string convertDoubleToStrScientific(double x, std::size_t max_width)
{
    std::size_t max_precision = std::min(max_width - 4, 14UL);
    std::string retval;
    std::stringstream ss;
    ss << std::scientific;
    ss << std::setprecision(max_precision);
    ss << x;
    retval            = ss.str();
    std::size_t pos_e = retval.find_last_of("eE");
    if (pos_e != std::string::npos)
    {
        double coeff;
        std::string s_sci_part = retval.substr(pos_e);
        std::string s_coeff    = retval.substr(0, pos_e);
        ss                     = std::stringstream(s_coeff);
        ss >> coeff;
        // remove trailing decimal zeros
        s_coeff.erase(s_coeff.find_last_not_of("0") + 1);
        // replace following line by this code to force whole number
        // coefficients to become #.0e+/-##, like 1e+10 will become 1.0e+10.
        // if (!s_coeff.empty() && s_coeff.back() == '.')
        //     s_coeff += "0";
        // otherwise, no decimal is added to whole number coefficients.
        s_coeff.erase(s_coeff.find_last_not_of(".") + 1);
        // manipulate coefficient to fit number into max width
        if (max_width < s_coeff.size() + s_sci_part.size())
        {
            if (max_width < s_sci_part.size() + 3)
                max_precision = 1;
            else
                max_precision = max_width - s_sci_part.size() - 2;
            ss = std::stringstream();
            ss << std::fixed;
            ss << std::setprecision(max_precision);
            ss << coeff;
            s_coeff = ss.str();
        } // end if
        // create the final scientific notation output
        if (s_coeff.empty() || coeff == 0.0)
        {
            retval = "0";
        } // end if
        else
        {
            retval = s_coeff + s_sci_part;
        } // end else
    } // end if
    else
    {
        // scientific notation not found (it should never be the case):
        // truncate trailing decimal zeros and do not modify the floating point output
        retval.erase(retval.find_last_not_of("0") + 1);
        retval.erase(retval.find_last_not_of(".") + 1);
    } // end else

    return retval;
}

void writeToFile(const std::string &filename, std::function<void(std::ostream &)> fn,
                 bool b_binary, bool b_append)
{
    std::fstream output_fnum;

    auto open_flags = (b_append ? std::fstream::app | std::fstream::ate : std::fstream::trunc);
    if (b_binary)
        open_flags |= std::fstream::binary;
    output_fnum.open(filename, std::fstream::out | open_flags);
    if (!output_fnum.is_open())
        throw std::ios_base::failure("Failed to open file \"" + filename + "\" for writing.");
    if (!output_fnum)
        throw std::ios_base::failure("Error after opening file \"" + filename + "\" for writing.");

    fn(output_fnum);

    output_fnum.close();
}

void writeToFile(const std::string &filename,
                 const char *p_data, std::size_t size,
                 bool b_binary, bool b_append)
{
    writeToFile(
        filename,
        [p_data, size](std::ostream &os) -> void {
            os.write(p_data, size);
        },
        b_binary, b_append);
}

//------------------------
// class CSVTokenizer
//------------------------

std::pair<std::string_view, std::string_view> CSVTokenizer::findNextValue(const std::string_view &s_line,
                                                                          char delim)
{
    std::pair<std::string_view, std::string_view> retval;
    std::string_view s_delim = std::string_view(&delim, 1);

    //if (s_delim.find_first_of(s_blanks) != s_delim.npos
    if (s_delim.find_first_of(hebench::Utilities::BlankTrim) != s_delim.npos
        || s_delim.find_first_of("\"") != s_delim.npos)
        throw std::invalid_argument("`delim` cannot be whitespace or quotation mark.");

    std::string_view &car = retval.first;
    std::string_view &cdr = retval.second;
    cdr                   = s_line;
    hebench::Utilities::trim(cdr);
    car = cdr;

    if (!cdr.empty()) // && cdr.front() != s_delim.at(0))
    {
        // find next comma
        auto comma_pos = cdr.find_first_of(s_delim);

        // check for quotations
        auto quote_pos = cdr.find_first_of("\"");
        while (comma_pos != cdr.npos
               && quote_pos != cdr.npos
               && quote_pos < comma_pos)
        {
            // comma was found inside the quotations

            // find closing quotations
            quote_pos = cdr.find_first_of("\"", quote_pos + 1);
            comma_pos = quote_pos == cdr.npos ?
                            cdr.npos :
                            cdr.find_first_of(s_delim, quote_pos + 1);
            // see if there are more quotations
            quote_pos = cdr.find_first_of("\"", quote_pos + 1);
        } // end while

        if (comma_pos != cdr.npos)
        {
            car.remove_suffix(car.size() - comma_pos);
            cdr.remove_prefix(comma_pos + 1);
        } // end if
        else
        {
            // there are no move values in cdr after car
            cdr = std::string_view();
        } // end else
    } // end if

    hebench::Utilities::trim(car);
    hebench::Utilities::trim(cdr);

    return retval;
}

std::vector<std::string_view> CSVTokenizer::tokenizeLineInPlace(const std::string_view &s_line, char delim)
{
    std::vector<std::string_view> retval;

    std::string_view cdr = s_line;
    hebench::Utilities::trim(cdr);

    // find first

    while (!cdr.empty())
    {
        auto car_cdr = findNextValue(cdr, delim);
        retval.push_back(car_cdr.first);
        cdr = car_cdr.second;
    } // end while

    return retval;
}

std::vector<std::string> CSVTokenizer::tokenizeLine(const std::string_view &s_line, char delim)
{
    auto tokens = tokenizeLineInPlace(s_line, delim);

    return std::vector<std::string>(tokens.begin(), tokens.end());
}

} // namespace Utilities
} // namespace hebench
