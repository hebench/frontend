// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __COMMON_Error_H_7e5fa8c2415240ea93eff148ed73539b
#define __COMMON_Error_H_7e5fa8c2415240ea93eff148ed73539b

#include <stdexcept>
#include <string>

namespace hebench {
namespace Common {

class ErrorException : public std::runtime_error
{
public:
    ErrorException(const std::string &msg = std::string(), int err_code = 0) :
        std::runtime_error(std::string()), m_err_code(err_code)
    {
        m_msg = msg;
        if (err_code != 0)
            m_msg += ("\nError code: " + std::to_string(m_err_code));
    }

    int getErrorCode() const { return m_err_code; }

    const char *what() const noexcept override { return m_msg.c_str(); }

private:
    std::string m_msg;
    int m_err_code;
};

} // namespace Common
} // namespace hebench

#endif // defined __COMMON_Error_H_7e5fa8c2415240ea93eff148ed73539b
