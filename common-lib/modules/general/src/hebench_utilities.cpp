
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

} // namespace Utilities
} // namespace hebench
