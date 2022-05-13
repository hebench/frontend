// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

//

#ifndef _ArgsParser_H_71409a03e9a84aa184b363b098e70fe9
#define _ArgsParser_H_71409a03e9a84aa184b363b098e70fe9

#include <cctype>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hebench {

/**
 * @brief Encapsulates the command line argument parsing using
 * C and C++ argument format.
 * @details Definitions:
 * Argument: command line argument.
 * Parameter: parameter modifying an argument.
 *
 * Example:
 * program.exe --input_file readme.txt --verbose --background_color 0 0.5 0.2
 * --invert_rgb
 *
 * Program name: program.exe
 * Arguments: --input_file, --verbose, --background_color
 * Parameters:
 *  For argument --input_file: readme.txt
 *  For argument --verbose: (none, flag argument)
 *  For argument --background_color: 0, 0.5, and 0.2
 *  For argument --invert_rgb: (none, flag argument)
 */
class ArgsParser
{
public:
    /**
     * @brief Instances of this class are thrown when help is requested.
     * @details This class derives from std::logic_error.
     */
    class HelpShown : public std::logic_error
    {
    public:
        explicit HelpShown(const std::string &msg) :
            std::logic_error(msg) {}
        virtual ~HelpShown() {}
    };
    /**
     * @brief Instances of this class are thrown when an invalid argument
     * is detected.
     * @details Invalid argument errors include unknown arguments, bad
     * formatting, missing positional arguments, etc.
     *
     * This class derives from std::logic_error.
     */
    class InvalidArgument : public std::logic_error
    {
    public:
        explicit InvalidArgument(const std::string &msg) :
            std::logic_error(msg) {}
        virtual ~InvalidArgument() {}
    };
    /**
     * @brief Instances of this class are thrown when there is a type
     * mismatch between a parameter passed and the requested type.
     * @details This class derives from std::bad_cast.
     */
    class InvalidType : public std::bad_cast
    {
    public:
        explicit InvalidType(const std::string &msg) :
            m_msg(msg) {}
        virtual ~InvalidType() {}
        const char *what() const noexcept override { return m_msg.c_str(); }

    private:
        std::string m_msg;
    };

    /**
     * @brief Constructs a new argument parser object.
     * @param bshow_help[in] If true, this object will automatically display
     * help manual to the standard output when the help flag is detected.
     * @param help_text[in] Help text to display before displaying the rest of
     * the help.
     * @param buse_exit[in] If true, std::exit(0) will be called when help is
     * requested, otherwise, exception HelpShown is thrown.
     * @details If \p bshow_help is true, help request is detected during the
     * call to parse(). The help flags that are checked for automatically are:
     * -h, /h, \\h, --help, /help, \help. If clients do not wish these flags to
     * be taken by the parser, make sure to set \p bshow_help to false, in which
     * case, clients will need to provide their own help message system.
     */
    ArgsParser(bool bshow_help = true, const std::string &help_text = std::string(), bool buse_exit = false);

    void printUsage() const;
    void printUsage(std::ostream &os) const;

    /**
     * @brief Adds a positional argument to this parser's list of positional arguments.
     * @param arg_name[in] Name of the positional argument.
     * @param help_text[in] Help text to be displayed for this argument.
     * @return The index for the positional argument in the list of positional arguments.
     * @details All positional arguments are required in the list of command line
     * arguments. Positional arguments must appear in order before any option arguments.
     * Failure to provide one of the positional arguments causes InvalidArgument
     * exception to be thrown during parsing.
     *
     * Parameters to this method are purely for display purposes when showing the help text.
     * The new positional argument is referenced by its index, as returned by this method.
     */
    std::size_t addPositionalArgument(const std::string &arg_name, const std::string &help_text = std::string());

    /**
     * @brief Adds an argument to this parser's list of parsable arguments.
     * @param arg[in] Name associated with this argument.
     * @param n[in] Number of parameters for this argument.
     * @param params_help[in] Help text to be displayed for parameters for this
     * argument when help is requested.
     * @param help_text[in] Help text to be displayed for this argument when
     * help is requested.
     * @sa addArgument(const std::initializer_list<std::string> &, std::size_t,
     * const std::string &, const std::string &)
     */
    void addArgument(const std::string &arg, std::size_t n = 0, const std::string &params_help = std::string(),
                     const std::string &help_text = std::string());
    /**
     * @brief Adds an argument to this parser's list of parsable arguments.
     * @param arg0[in] First name associated with this argument.
     * @param arg1[in] Second name associated with this argument.
     * @param n[in] Number of parameters for this argument.
     * @param params_help[in] Help text to be displayed for parameters for this
     * argument when help is requested.
     * @param help_text[in] Help text to be displayed for this argument when
     * help is requested.
     * @sa addArgument(const std::initializer_list<std::string> &, std::size_t,
     * const std::string &, const std::string &)
     */
    void addArgument(const std::string &arg0, const std::string &arg1, std::size_t n = 0, const std::string &params_help = std::string(),
                     const std::string &help_text = std::string());
    /**
     * @brief Adds an argument to this parser's list of parsable arguments.
     * @param arg0[in] First name associated with this argument.
     * @param arg1[in] Second name associated with this argument.
     * @param arg3[in] Third name associated with this argument.
     * @param n[in] Number of parameters for this argument.
     * @param params_help[in] Help text to be displayed for parameters for this
     * argument when help is requested.
     * @param help_text[in] Help text to be displayed for this argument when
     * help is requested.
     * @sa addArgument(const std::initializer_list<std::string> &, std::size_t,
     * const std::string &, const std::string &)
     */
    void addArgument(const std::string &arg0, const std::string &arg1, const std::string &arg2, std::size_t n = 0,
                     const std::string &params_help = std::string(), const std::string &help_text = std::string());
    /**
     * @brief Adds an argument to this parser's list of parsable arguments.
     * @param args[in] List of strings representing all the names associated
     * with this argument.
     * @param n[in] Number of parameters for this argument.
     * @param params_help[in] Help text to be displayed for parameters for this
     * argument when help is requested.
     * @param help_text[in] Help text to be displayed for this argument when
     * help is requested.
     * @sa addArgument(const std::initializer_list<std::string> &, std::size_t,
     * const std::string &, const std::string &)
     */
    void addArgument(const std::initializer_list<std::string> &args, std::size_t n = 0, const std::string &params_help = std::string(),
                     const std::string &help_text = std::string());
    /**
     * @brief Adds an argument to this parser's list of parsable arguments.
     * @param args[in] Vector of strings representing all the names associated
     * with this argument.
     * @param n[in] Number of parameters for this argument.
     * @param params_help[in] Help text to be displayed for parameters for this
     * argument when help is requested.
     * @param help_text[in] Help text to be displayed for this argument when
     * help is requested.
     * @details Once added, this argument can be referenced in all methods that
     * require an argument name by all the possible names passed in vector \p
     * args.
     *
     * If \p n is 0, then this is considered a flag argument. Flag arguments
     * have no parameters and are basically checked whether they have been
     * passed or not.
     *
     * When \p n is not 0, that many parameters will be parsed from the command
     * line. If not enough parameters are passed, an error is reported when
     * trying to parse more parameters than supplied or the next argument parsed
     * is invalid. However, parsing parameters like this can skip other
     * arguments when the command line is ill-constructed. For example, if
     * argument --background_color takes 3 parameters, but the command line is
     * as follows: program.exe --background_color 0 0.5 --verbose, the --verbose
     * flag will be pushed into the third parameter of --background_color
     * argument and no error will be detected until attempting to retrieve the
     * third argument of --background_color.
     */
    void addArgument(const std::vector<std::string> &args, std::size_t n = 0, const std::string &params_help = std::string(),
                     const std::string &help_text = std::string());

    /**
     * @brief Parses a command line argument list.
     * @param argc[in] Number of arguments in the list.
     * @param argv[in] List of command line arguments.
     * @param start_index[in] Index in argument list where to start parsing.
     * @throws std::logic_error if  an argument is not valid (it is not parsable
     * by this parser), or the number of parameters for an argument or positional
     * arguments are insufficient.
     * @throws ArgsParser::HelpShown if buse_help and buse_exit were set during
     * construction, and a help flag was parsed in the list of arguments.
     */
    void parse(int argc, char *const argv[], int start_index = 1);

    /**
     * @brief Retrieves a vector of strings representing the values passed as
     * parameters to an argument.
     * @param arg[in] Argument for which to retrieve the values.
     * @return Vector of strings containing the values passed as to the argument
     * specified.
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     * @sa getValue(const std::string &).
     */
    const std::vector<std::string> &operator[](const std::string &arg) const { return getValue(arg); }
    /**
     * @brief Tests whether an argument is parsable by this parser.
     * @param arg[in] Argument to test.
     * @retval true if argument can be parsed.
     * @retval false otherwise.
     */
    bool isArgumentValid(const std::string &arg) const;
    /**
     * @brief Tests whether the specified argument was found during parsing of
     * the command line.
     * @param arg[in] Argument to find.
     * @retval true if argument was parsed from the command line.
     * @retval false otherwise.
     * @details This method is usually invoked to determine whether flag
     * arguments have been set or not. For example, if an argument was added as
     * a flag argument using the following:
     * @code
     * parser.addArgument("--inver_rgb", 0); // --inver_rgb argument is a flag
     * argument because
     *                                       // it takes no parameters
     * @endcode
     * Then, we can test whether the flag argument was set in the command line
     * as such:
     * @code
     * if (parser.hasArgument("--inver_rgb"))
     *     std::cout << "BGR"; // invert RGB
     * else
     *     std::cout << "RGB"; // do not invert RGB
     * @endcode
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     */
    bool hasArgument(const std::string &arg) const;
    /**
     * @brief Tests whether an argument had value passed through the command
     * line.
     * @param arg[in] Argument to test.
     * @retval true if argument has a value parsed from the command line.
     * @retval false otherwise.
     * @details This is true if there was a value passed, for example, if
     * argument for file name is set, it could be:
     * --input_file readme.txt
     * In this case, value for --input_file is readme.txt. This method would
     * return true.
     *
     * If an argument has the form:
     * --verbose
     * then this method returns false because no value was set for parameter
     * --verbose.
     *
     * If \p arg has not been parsed from the command line, this method returns
     * false.
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     */
    bool hasValue(const std::string &arg) const;
    /**
     * @brief Retrieves a vector of strings representing the values passed as
     * parameters to an argument.
     * @param arg[in] Argument for which to retrieve the values.
     * @return Vector of strings containing the values passed as to the argument
     * specified.
     * @details The vector returned will have as many elements as parameters are
     * required for the argument \p arg. If some of the arguments were not
     * assigned values during parsing, then they will be empty strings.
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     */
    const std::vector<std::string> &getValue(const std::string &arg) const;

    template <class T>
    /**
     * @brief Retrieves the value at the specified index for the specified
     * argument.
     * @param value[out] Reference to variable where to store the result. It
     * must be copy assignable.
     * @param arg[in] Argument for which to retrieve the value.
     * @param param_index[in] Zero-based index for the argument.
     * @details This method retrieves the value at a specified index of an
     * argument. For example, if an argument is added such as this:
     * @code
     * // add a background color parameter that takes 3 arguments
     * // to be understood as red green and blue
     * parser.addArgument("--background_color", 3, "R G B");
     * @endcode
     * Then, we can retrieve the color values as follows:
     * @code
     * double r, g, b;
     * parser.getValueAt<double>(r, "--background_color", 0); // OK
     * parser.getValueAt<double>(g, "--background_color", 1); // OK
     * parser.getValueAt<double>(b, "--background_color", 2); // OK
     *
     * double t;
     * parser.getValueAt<double>(t, "--background_color", 3); // error: only 3
     * parameters available.
     * @endcode
     *
     * Name of the argument specified by \p arg can be any of the names
     * associated with a parameter as defined during the respective call to
     * addArgument().
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     * @throws std::out_of_range if \p param_index is beyond the last index of
     * the values for the specified argument.
     * @throw std::bad_cast if value passed to specified argument's parameter is
     * not of the correct type.
     */
    void getValueAt(T &value, const std::string &arg, std::size_t param_index) const
    {
        if (!hasValue(arg))
            throw std::logic_error("No value set for argument \"" + arg + "\".");
        if (param_index >= this->getValue(arg).size())
            throw std::out_of_range("\"param_index\" is out of range.");
        const std::string &s = this->getValue(arg).at(param_index);
        std::istringstream ss(s);
        if (!(ss >> value))
            throw InvalidType("Invalid value for argument \"" + arg + "\": \"" + s + "\"");
    }

    template <class T>
    /**
     * @brief Retrieves the value for the specified argument.
     * @param value[out] Reference to variable where to store the result. It
     * must be copy assignable.
     * @param arg[in] Argument for which to retrieve the value.
     * @details This method retrieves the value of an argument.
     *
     * Name of the argument specified by \p arg can be any of the names
     * associated with a parameter as defined during the respective call to
     * addArgument().
     *
     * If the argument has more than one parameter, this method returns the
     * value of the first parameter.
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser or if argument does not have a value.
     */
    void getValue(T &value, const std::string &arg) const
    {
        this->getValueAt(value, arg, 0);
    }

    template <class T>
    /**
     * @brief Retrieves the value at the specified index for the specified
     * argument, returning a default value if no argument or value is found.
     * @param value[out] Reference to variable where to store the result. It
     * must be copy assignable.
     * @param arg[in] Argument for which to retrieve the value.
     * @param param_index[in] Zero-based index for the argument.
     * @param default_value[in] Default value to use if parameter was not passed
     * or no value has been set for the argument.
     * @details This method retrieves the value at a specified index of an
     * argument. For example, if an argument is added such as this:
     * @code
     * // add a background color parameter that takes 3 arguments
     * // to be understood as red green and blue
     * parser.addArgument("--background_color", 3, "R G B");
     * @endcode
     * Then, we can retrieve the color values as follows:
     * @code
     * double r, g, b;
     * parser.getValueAt<double>(r, "--background_color", 0, 0.0); // use
     * default value 0.0 if no "--background_color" argument was passed.
     * parser.getValueAt<double>(g, "--background_color", 1, 0.0);
     * parser.getValueAt<double>(b, "--background_color", 2, 0.0);
     * @endcode
     *
     * Name of the argument specified by \p arg can be any of the names
     * associated with a parameter as defined during the respective call to
     * addArgument().
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     * @throws std::out_of_range if \p param_index is beyond the last index of
     * the values for the specified argument.
     * @throw std::bad_cast if parameter passed to argument is not of the
     * specified type.
     */
    void getValueAt(T &value, const std::string &arg, std::size_t param_index, const T &default_value) const
    {
        findArgID(arg);
        if (param_index >= this->getValue(arg).size())
            throw std::out_of_range("\"param_index\" is out of range. No parameter with that index "
                                    "for specified argument.");
        try
        {
            this->getValueAt<T>(value, arg, param_index);
        }
        catch (std::bad_cast &)
        {
            throw;
        }
        catch (...)
        {
            value = default_value;
        }
    }

    template <class T>
    /**
     * @brief Retrieves the value for the specified argument,
     * returning a default value if no argument or value is found.
     * @param value[out] Reference to variable where to store the result. It
     * must be copy assignable.
     * @param arg[in] Argument for which to retrieve the value.
     * @param default_value[in] Default value to use if parameter was not passed
     * or no value has been set for the argument.
     * @details This method retrieves the value of an argument.
     *
     * Name of the argument specified by \p arg can be any of the names
     * associated with a parameter as defined during the respective call to
     * addArgument().
     *
     * If the argument has more than one parameter, this method returns the
     * value of the first parameter.
     * @throws std::logic_error if \p arg is not an argument parsable by this
     * parser.
     * @throws std::out_of_range if \p param_index is beyond the last index of
     * the values for the specified argument.
     * @throw std::bad_cast if parameter passed to argument is not of the
     * specified type.
     */
    void getValue(T &value, const std::string &arg, const T &default_value) const
    {
        this->getValueAt(value, arg, 0, default_value);
    }

    /**
     * @brief Retrieves the string containing the value passed as parameter to
     * a positional argument.
     * @param arg_position[in] Zero-based position index of positional argument
     * to retrieve.
     * @return String containing the value passed as parameters to
     * the specified positional argument.
     * @details \p arg_position is the index corresponding to a positional argument
     * and it the same value returned when adding the positional argument to this
     * parser.
     * @throws std::out_of_range if \p arg_position is not a valid index for a
     * posional argument in this parser.
     */
    const std::string &getPositionalValue(std::size_t arg_position) const;

    template <class T>
    /**
     * @brief Retrieves the the value passed as parameter to a positional argument.
     * @param value[out] Reference to variable where to store the result. It
     * must be copy assignable.
     * @param arg_position[in] Zero-based position index of positional argument
     * to retrieve.
     * @details \p arg_position is the index corresponding to a positional argument
     * and it the same value returned when adding the positional argument to this
     * parser.
     * @throws std::out_of_range if \p arg_position is not a valid index for a
     * posional argument in this parser.
     * @throw std::bad_cast if parameter passed to argument is not of the
     * specified type.
     */
    void getPositionalValue(T &value, std::size_t arg_position) const
    {
        const std::string &s = this->getPositionalValue(arg_position);
        std::istringstream ss(s);
        if (!(ss >> value))
            throw InvalidType("Invalid value for positional argument \"" + std::to_string(arg_position) + "\": \"" + s + "\"");
    }

    /**
     * @brief Retrieves the number of non-positional arguments parsed from the
     * command line.
     * @return Number of non-positional arguments parsed by last call to parse().
     */
    std::size_t count() const { return m_set_args.size(); }
    /**
     * @brief Returns true if no non-positional arguments were parsed from the
     * command line.
     */
    std::size_t empty() const { return m_set_args.empty(); }
    /**
     * @brief Retrieves the number of positional arguments required by this
     * parser.
     */
    std::size_t count_positional() const { return m_positional_values.size(); }

private:
    typedef std::size_t args_unique_id;

    void add(const std::vector<std::string> &args, std::size_t n, const std::string &params_help, const std::string &help_text);
    args_unique_id findArgID(const std::string &arg) const;
    bool checkShowHelp(args_unique_id id);
    void showHelp() const;
    void showHelp(std::ostream &os) const;
    bool hasArgument(args_unique_id id) const;

    bool m_buse_exit;
    args_unique_id m_help_id;
    std::string m_program_name;
    std::string m_help_text;
    std::unordered_map<std::string, args_unique_id> m_map_args; // maps args to unique id (all arguments parsable by this
    // parser)
    std::unordered_map<args_unique_id, std::vector<std::string>> m_map_values; // maps id to value
    std::map<std::string, std::vector<std::string>> m_map_help; // help text
    std::unordered_set<args_unique_id> m_set_args; // set of arguments actually passed
    std::vector<std::pair<std::string, std::string>> m_positional_args; // each element is a pair ( "arg_name", "arg_description" )
        // for help purposes
    std::vector<std::string> m_positional_values; // contains the values for the positional arguments
};

template <>
inline void ArgsParser::getValueAt<std::string>(std::string &value, const std::string &arg, std::size_t param_index) const
{
    if (!hasValue(arg))
        throw std::logic_error("No value set for argument \"" + arg + "\".");
    if (param_index >= this->getValue(arg).size())
        throw std::out_of_range("\"param_index\" is out of range.");
    value = this->getValue(arg).at(param_index);
}

template <>
inline void ArgsParser::getValueAt<bool>(bool &value, const std::string &arg, std::size_t param_index) const
{
    std::string s_value;
    bool bmatch; // matched a false
    const std::string s_0     = "0";
    const std::string s_false = "false";
    getValueAt<std::string>(s_value, arg, param_index);

    bmatch = s_value == s_0;
    if (!bmatch)
    {
        bmatch = true;
        for (std::size_t i = 0; bmatch && i < s_value.size() && i < s_false.size(); i++)
            bmatch = s_false[i] == std::tolower(s_value[i]);
    }

    value = !bmatch;
}

} // namespace hebench

#endif
