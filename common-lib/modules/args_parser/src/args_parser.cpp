// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "../include/args_parser.h"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace hebench;

ArgsParser::ArgsParser(bool bshow_help, const std::string &help_text, bool buse_exit) :
    m_buse_exit(buse_exit), m_help_id(std::numeric_limits<std::size_t>::max()), m_help_text(help_text)
{
    if (bshow_help)
    {
        addArgument({ "-h", "/h", "\\h", "--help", "/help", "\\help" }, 0, std::string(), "    Shows this help.");
        m_help_id = findArgID("-h");
    }
}

std::size_t ArgsParser::addPositionalArgument(const std::string &arg_name, const std::string &help_text)
{
    m_positional_args.emplace_back(arg_name, help_text);
    m_positional_values.resize(m_positional_args.size());
    return m_positional_values.size() - 1;
}

void ArgsParser::addArgument(const std::string &arg, std::size_t n, const std::string &params_help, const std::string &help_text)
{
    add({ arg }, n, params_help, help_text);
}

void ArgsParser::addArgument(const std::string &arg0, const std::string &arg1, std::size_t n, const std::string &params_help,
                             const std::string &help_text)
{
    add({ arg0, arg1 }, n, params_help, help_text);
}

void ArgsParser::addArgument(const std::string &arg0, const std::string &arg1, const std::string &arg2, std::size_t n,
                             const std::string &params_help, const std::string &help_text)
{
    add({ arg0, arg1, arg2 }, n, params_help, help_text);
}

void ArgsParser::addArgument(const std::initializer_list<std::string> &args, std::size_t n, const std::string &params_help,
                             const std::string &help_text)
{
    add(args, n, params_help, help_text);
}

void ArgsParser::addArgument(const std::vector<std::string> &args, std::size_t n, const std::string &params_help,
                             const std::string &help_text)
{
    add(args, n, params_help, help_text);
}

void ArgsParser::parse(int argc, char *const argv[], int start_index)
{
    if (start_index < 0)
        start_index = 0;
    // get program name if available
    if (start_index > 0)
    {
        m_program_name              = argv[0];
        std::size_t separator_index = m_program_name.find_last_of('/');
        if (separator_index == std::string::npos)
            separator_index = m_program_name.find_last_of('\\');
        if (separator_index != std::string::npos)
            m_program_name = m_program_name.substr(separator_index + 1);
    }
    if (argc < start_index)
        throw std::invalid_argument("Not enough arguments.");
    if (static_cast<std::size_t>(argc - start_index) < count_positional())
        throw InvalidArgument("Insufficient positional parameters.");

    std::size_t positional_arg_i = 0;
    int i                        = start_index;
    while (i < argc)
    {
        std::string sarg = argv[i];

        if (positional_arg_i < count_positional())
        {
            try
            {
                // check if argument is help
                args_unique_id id = findArgID(sarg);
                if (checkShowHelp(id))
                    i = argc;
            }
            catch (InvalidArgument &)
            {
                // parse positional argument
                m_positional_values[positional_arg_i++] = sarg;
                ++i;
            }
        } // end if
        else
        {
            args_unique_id id = findArgID(sarg);
            if (checkShowHelp(id))
            {
                i = argc;
            } // end if
            else
            {
                // parse argument

                m_set_args.insert(id); // argument passed
                // check if it requires values
                if (!m_map_values[id].empty())
                {
                    std::vector<std::string> &values = m_map_values[id];
                    if (i + static_cast<int>(values.size()) >= argc)
                        throw std::logic_error("Insufficient number of parameters for argument \"" + sarg + "\".");
                    for (std::size_t j = 0; j < values.size(); ++j)
                        values[j] = argv[i + j + 1];
                    i += static_cast<int>(values.size());
                } // end if
                ++i;
            } // end else
        } // end else
    } // end while
}

bool ArgsParser::isArgumentValid(const std::string &arg) const
{
    return m_map_args.count(arg) > 0;
}

bool ArgsParser::hasArgument(const std::string &arg) const
{
    return hasArgument(findArgID(arg));
}

bool ArgsParser::hasArgument(args_unique_id id) const
{
    return m_set_args.count(id) > 0;
}

bool ArgsParser::hasValue(const std::string &arg) const
{
    args_unique_id id = findArgID(arg);
    return hasArgument(id) && !m_map_values.at(id).empty();
}

const std::vector<std::string> &ArgsParser::getValue(const std::string &arg) const
{
    return m_map_values.at(findArgID(arg));
}

const std::string &ArgsParser::getPositionalValue(std::size_t arg_position) const
{
    return m_positional_values.at(arg_position);
}

void ArgsParser::add(const std::vector<std::string> &args, std::size_t n, const std::string &params_help, const std::string &help_text)
{
    if (args.empty())
        throw std::invalid_argument("Invalid empty arguments.");
    args_unique_id id        = m_map_values.size();
    std::string help_text_id = args.front();
    std::vector<std::string> arr_help_text;
    arr_help_text.reserve(args.size() + 3);
    // map all parsable argument names for this argument to a single ID
    for (const std::string &s : args)
    {
        if (m_map_args.count(s) > 0)
            throw std::invalid_argument("Invalid duplicated argument: \"" + s + "\".");
        m_map_args[s] = id;
        arr_help_text.emplace_back(s);
    }
    arr_help_text.emplace_back();
    arr_help_text.emplace_back(params_help);
    arr_help_text.emplace_back(help_text);
    m_map_help[help_text_id] = arr_help_text;

    m_map_values[id] = std::vector<std::string>(n);
}

ArgsParser::args_unique_id ArgsParser::findArgID(const std::string &arg) const
{
    if (!isArgumentValid(arg))
        throw InvalidArgument("Invalid argument: \"" + arg + "\".");
    return m_map_args.at(arg);
}

void ArgsParser::printUsage() const
{
    printUsage(std::cout);
}

void ArgsParser::printUsage(std::ostream &os) const
{
    os << "Usage:" << std::endl
       << "    ";
    m_program_name.empty() ? os << "program" : os << m_program_name;
    if (count_positional() > 0)
    {
        for (std::size_t i = 0; i < count_positional(); ++i)
            os << " " << m_positional_args[i].first;
    } // end if
    if (!m_map_help.empty())
        os << " OPTIONS";
    os << std::endl;
}

bool ArgsParser::checkShowHelp(args_unique_id id)
{
    bool retval = id == m_help_id;
    if (retval)
    {
        m_map_args.clear();
        m_map_values.clear();
        m_set_args.clear();
        showHelp();
    } // end if

    return retval;
}

void ArgsParser::showHelp() const
{
    showHelp(std::cout);
}

void ArgsParser::showHelp(std::ostream &os) const
{
    // build and display the help text
    if (!m_help_text.empty())
        os << m_help_text << std::endl
           << std::endl;
    printUsage(os);
    if (count_positional() > 0)
    {
        os << std::endl
           << "POSITIONAL ARGUMENTS: " << count_positional() << std::endl;
        for (std::size_t i = 0; i < count_positional(); ++i)
        {
            os << m_positional_args[i].first << std::endl
               << m_positional_args[i].second << std::endl
               << std::endl;
        } // end for
    } // end if
    if (!m_map_help.empty())
    {
        os << std::endl
           << "OPTIONS:" << std::endl;
        for (auto help_data : m_map_help)
        {
            const std::vector<std::string> &arr_help_data = help_data.second;
            if (!arr_help_data.back().empty())
            {
                int help_idx    = -1;
                bool bhelp_text = false;
                for (std::size_t i = 0; i < arr_help_data.size(); i++)
                {
                    // if true, then help text is comming next
                    if (!bhelp_text && arr_help_data[i].empty())
                    {
                        help_idx   = 0;
                        bhelp_text = true; // help text coming next
                    } // end if
                    else
                    {
                        if (help_idx < 0 && i > 0)
                            os << ", ";
                        else if (help_idx == 0)
                            os << " ";
                        os << arr_help_data[i];
                        if (help_idx == 0)
                        {
                            os << std::endl;
                            help_idx = 1;
                        } // end if
                    } // end else
                } // end for
                os << std::endl
                   << std::endl;
            } // end if
        } // end for
    } // end if

    if (m_buse_exit)
        std::exit(0);
    else
        throw HelpShown("Help requested.");
}
