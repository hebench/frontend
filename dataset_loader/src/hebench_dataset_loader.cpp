// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "hebench/modules/general/include/hebench_utilities.h"
#include "hebench_dataset_loader.h"

namespace hebench {
namespace DataLoader {

class EDLHelper
{
public:
    enum ControlLine : std::uint64_t
    {
        Index_ControlIdentifier = 0,
        Index_ControlComponentIndex,
        Index_ControlNumSamples,
        Index_ControlKind,
        ControlTokenSize, // number of mandatory tokens in the control line
        Index_ControlPadToLength = ControlTokenSize,
        Index_ControlPadValue,
        ControlMaxTokenSize, // maximum number of tokens in the control line
    };

    enum CSVLine : std::uint64_t
    {
        Index_CSVFilename = 0,
        CSVTokenSize, // number of mandatory tokens in the CSV line
        Index_CSVFromLine = CSVTokenSize,
        Index_CSVNumSamples,
        CSVMaxTokenSize // maximum number of tokens in the CSV line
    };

    constexpr static const char *ControlLineInput     = "input";
    constexpr static const char *ControlLineOutput    = "output";
    constexpr static const char *ControlLineKindLocal = "local";
    constexpr static const char *ControlLineKindCSV   = "csv";

public:
    static bool isComment(std::string_view s_view);
    static bool isEmpty(std::string_view s_view);
    static bool isCommentOrEmpty(std::string_view s_view) { return isComment(s_view) || isEmpty(s_view); }
};

bool EDLHelper::isComment(std::string_view s_view)
{
    hebench::Utilities::ltrim(s_view);
    return !s_view.empty() && s_view.front() == '#';
}

bool EDLHelper::isEmpty(std::string_view s_view)
{
    hebench::Utilities::trim(s_view);
    return s_view.empty();
}

template <typename T>
class EDLTypedHelper
{
private:
    std::uint64_t m_pad_to_length;
    T m_pad_value;
    std::filesystem::path m_working_path;

public:
    /**
     * @brief Reads the next data block as described by the control line
     * from the specified stream.
     * @param[out] out_data Structure where to store the read data.
     * @param is Input data stream.
     * @param[in] control_line_tokens Control line defining the data block.
     * @param[in] working_path The current working directory. Any relative
     * paths will be resolved relative to this.
     * @param[in,out] line_num Increases every time a line is extracted from
     * the input stream \p is.
     * @throws Standard C++ exception derived from std::exception on error describing the
     * failure.
     * @return The number of lines extracted from the input stream.
     * @details This method returns the number of lines extracted from the
     * input stream to complete the reading of the data block. Note that this
     * counts empty lines and lines containing comments.
     *
     * The stream reading pointer will be advanced to the next line after the
     * data block.
     */
    static void readDataBlock(std::vector<std::vector<std::vector<T>>> &out_data,
                              std::istream &is,
                              const std::vector<std::string_view> &control_line_tokens,
                              const std::filesystem::path &working_path,
                              std::uint64_t &line_num);

private:
    /**
     * @brief Constructs a new EDLTypedHelper object.
     * @param[in] pad_to_length Enables padding, up to this number of items.
     * For more information see @ref dataset_csv_reference .
     * @param[in] pad_value Value to use for padding.
     * @param[in] working_path The current working directory. Any relative
     * paths will be resolved relative to this.
     */
    EDLTypedHelper(std::uint64_t pad_to_length,
                   const T &pad_value,
                   const std::filesystem::path &working_path);

    /**
     * @brief Reads data block as defined by `local` kind.
     * @param[out] out_data Collection where to store the read samples.
     * @param is Input data stream.
     * @param[in] line_offset Line offset inside the input stream from where to start reading.
     * Line offset is relative to the current reading position in the stream.
     * @param[in] num_samples Number of samples to read.
     * @param[in,out] line_num Increases every time a line is extracted from
     * the input stream \p is.
     * @throws Standard C++ exception derived from std::exception on error describing the
     * failure.
     * @return The number of lines extracted from the input stream.
     * @details This method returns the number of lines extracted from the
     * input stream to complete the reading of the data block. Note that this
     * counts empty lines and lines containing comments.
     *
     * The stream reading pointer will be advanced to the next line after the
     * data block.
     *
     * This method will read samples from the stream, a sample per line, until the
     * number of samples requested is read, or the end of the stream is found. To
     * read all the samples left in the stream, pass `std::numeric_limits<std::uint64_t>::max()`
     * into \p num_samples .
     */
    void readLocalDataBlock(std::vector<std::vector<T>> &out_data,
                            std::istream &is,
                            std::uint64_t line_offset,
                            std::uint64_t num_samples,
                            std::uint64_t &line_num);
    /**
     * @brief Reads data block as defined by `csv` kind.
     * @param[out] out_data Collection where to store the read samples.
     * @param is Input data stream.
     * @param[in] line_offset Line offset inside the input stream from where to start reading.
     * Line offset is relative to the current reading position in the stream.
     * @param[in] num_samples Number CSV of samples to read from input stream.
     * @param[in,out] line_num Increases every time a line is extracted from
     * the input stream \p is.
     * @throws Standard C++ exception derived from std::exception on error describing the
     * failure.
     * @return The number of lines extracted from the input stream.
     * @details This method returns the number of lines extracted from the
     * input stream to complete the reading of the data block. Note that this
     * counts empty lines and lines containing comments.
     *
     * The stream reading pointer will be advanced to the next line after the
     * data block.
     *
     * This method will read CSV samples from the stream, a sample per line, until the
     * number of CSV samples requested is read, or the end of the stream is found. To
     * read all the samples left in the stream, pass `std::numeric_limits<std::uint64_t>::max()`
     * into \p num_samples .
     *
     * Note that a CSV sample points to a CSV file that may contain one or more data samples.
     */
    void readCSVDataBlock(std::vector<std::vector<T>> &out_data,
                          std::istream &is,
                          std::uint64_t line_offset,
                          std::uint64_t num_samples,
                          std::uint64_t &line_num);

    /**
     * @brief Parses a local data sample from a CSV line.
     * @param[in] s_csv_line_sample Line from which to extract the data sample.
     * @throws Standard C++ exception derived from std::exception on error describing the
     * failure.
     * @return Vector containing the data sample, where each element is a scalar of
     * type template `T`, parsed from the input CSV line.
     * @details Tokens from the CSV line that cannot be directly read into a value of
     * type `T` will be converted to string and read as ASCII values, one element per
     * byte. Multibyte strings should work normally as each byte in a multibyte character
     * will be saved.
     */
    std::vector<T> parseDataSample(std::string_view s_csv_line_sample);
    /**
     * @brief Parses a token value as a string, turning it into
     * equivalent ASCII values.
     * @param[in] sv_token String to parse.
     * @return A vector of elements of template type `T` where each element corresponds
     * to a character ASCII value from the input string, cast into `T`.
     * @details Quotations surrounding the input will be ignored. This is, for example,
     * if input is `"foo"`, the return value will be [ 102, 111, 111 ], where the leading
     * and ending quations are ignored.
     *
     * Leading and ending blankspaces (outside of quotations) will also be ignored.
     */
    static std::vector<T> parseTokenAsString(std::string_view sv_token);
};

template <typename T>
void EDLTypedHelper<T>::readDataBlock(std::vector<std::vector<std::vector<T>>> &out_data,
                                      std::istream &is,
                                      const std::vector<std::string_view> &control_line_tokens,
                                      const std::filesystem::path &working_path,
                                      std::uint64_t &line_num)
{
    assert(control_line_tokens.size() >= EDLHelper::ControlLine::ControlTokenSize);
    assert(is.good());
    assert(std::filesystem::exists(working_path) && std::filesystem::is_directory(working_path));

    // parse the control line

    std::uint64_t component_index;
    std::uint64_t num_samples;
    std::uint64_t pad_to_length = 0;
    T pad_value                 = static_cast<T>(0);
    std::stringstream ss;
    std::string err_msg;

    err_msg.clear();
    try
    {
        std::string s_component_index(control_line_tokens[EDLHelper::ControlLine::Index_ControlComponentIndex]);
        ss = std::stringstream(s_component_index);
        if (!(ss >> component_index))
            throw std::runtime_error("Invalid value \"" + s_component_index + "\".");
    }
    catch (std::exception &ex)
    {
        ss = std::stringstream();
        ss << ": " << ex.what();
        err_msg = ss.str();
    }
    catch (...)
    {
        err_msg = ".";
    }
    if (!err_msg.empty())
    {
        ss = std::stringstream();
        ss << "Error reading <component_index> from control line" << err_msg;
        throw std::runtime_error(ss.str());
    } // end if

    err_msg.clear();
    try
    {
        std::string s_num_samples(control_line_tokens[EDLHelper::ControlLine::Index_ControlNumSamples]);
        ss = std::stringstream(s_num_samples);
        if (!(ss >> num_samples))
            throw std::runtime_error("Invalid value \"" + s_num_samples + "\".");
    }
    catch (std::exception &ex)
    {
        ss = std::stringstream();
        ss << ": " << ex.what();
        err_msg = ss.str();
    }
    catch (...)
    {
        err_msg = ".";
    }
    if (!err_msg.empty())
    {
        ss = std::stringstream();
        ss << "Error reading <num_samples> from control line" << err_msg;
        throw std::runtime_error(ss.str());
    } // end if

    err_msg.clear();
    try
    {
        if (control_line_tokens.size() > EDLHelper::ControlLine::Index_ControlPadToLength)
        {
            std::string s_pad_to_len(control_line_tokens[EDLHelper::ControlLine::Index_ControlPadToLength]);
            ss = std::stringstream(s_pad_to_len);
            if (!(ss >> pad_to_length))
                throw std::runtime_error("Invalid value \"" + s_pad_to_len + "\".");
        } // end if
    }
    catch (std::exception &ex)
    {
        ss = std::stringstream();
        ss << ": " << ex.what();
        err_msg = ss.str();
    }
    catch (...)
    {
        err_msg = ".";
    }
    if (!err_msg.empty())
    {
        ss = std::stringstream();
        ss << "Error reading <pad_to_length> from control line" << err_msg;
        throw std::runtime_error(ss.str());
    } // end if

    err_msg.clear();
    try
    {
        if (control_line_tokens.size() > EDLHelper::ControlLine::Index_ControlPadValue)
        {
            std::string s_pad_value(control_line_tokens[EDLHelper::ControlLine::Index_ControlPadValue]);
            ss = std::stringstream(s_pad_value);
            if (!(ss >> pad_value))
                throw std::runtime_error("Invalid value \"" + s_pad_value + "\".");
        } // end if
    }
    catch (std::exception &ex)
    {
        ss = std::stringstream();
        ss << ": " << ex.what();
        err_msg = ss.str();
    }
    catch (...)
    {
        err_msg = ".";
    }
    if (!err_msg.empty())
    {
        ss = std::stringstream();
        ss << "Error reading <pad_value> from control line" << err_msg;
        throw std::runtime_error(ss.str());
    } // end if

    while (out_data.size() <= component_index)
        out_data.emplace_back(std::vector<std::vector<T>>());

    EDLTypedHelper<T> helper(pad_to_length, pad_value, working_path);

    // identify `kind` of data: `local` or `csv`
    if (control_line_tokens[EDLHelper::ControlLine::Index_ControlKind] == EDLHelper::ControlLineKindLocal)
    {
        helper.readLocalDataBlock(out_data[component_index],
                                  is, 0, num_samples,
                                  line_num);
    } // end if
    else if (control_line_tokens[EDLHelper::ControlLine::Index_ControlKind] == EDLHelper::ControlLineKindCSV)
    {
        helper.readCSVDataBlock(out_data[component_index],
                                is, 0, num_samples,
                                line_num);
    } // end else if
    else
    {
        ss = std::stringstream();
        ss << "Invalid control line kind: \"" << control_line_tokens[EDLHelper::ControlLine::Index_ControlKind] << "\".";
        throw std::runtime_error(ss.str());
    } // end else
}

template <typename T>
EDLTypedHelper<T>::EDLTypedHelper(std::uint64_t pad_to_length,
                                  const T &pad_value,
                                  const std::filesystem::path &working_path) :
    m_pad_to_length(pad_to_length),
    m_pad_value(pad_value)
{
    m_working_path = std::filesystem::canonical(working_path);
}

template <typename T>
void EDLTypedHelper<T>::readLocalDataBlock(std::vector<std::vector<T>> &out_data,
                                           std::istream &is,
                                           std::uint64_t line_offset,
                                           std::uint64_t num_samples,
                                           std::uint64_t &line_num)
{
    assert(is.good());

    std::uint64_t samples_read = 0;

    // skip the line offset
    while (is && line_num < line_offset)
    {
        std::string s_line;
        if (std::getline(is, s_line))
            ++line_num;
    } // end while

    // read samples until we reach number of samples requested or end of file
    while (is && samples_read < num_samples)
    {
        std::string s_line;
        if (std::getline(is, s_line))
        {
            ++line_num;
            if (!EDLHelper::isCommentOrEmpty(s_line))
            {
                auto sample = parseDataSample(s_line);
                out_data.emplace_back(std::move(sample));

                ++samples_read;
            } // end if
        } // end if
    } // end while
}

template <typename T>
void EDLTypedHelper<T>::readCSVDataBlock(std::vector<std::vector<T>> &out_data,
                                         std::istream &is,
                                         std::uint64_t line_offset,
                                         std::uint64_t num_samples,
                                         std::uint64_t &line_num)
{
    assert(is.good());

    std::string err_msg;
    std::uint64_t samples_read = 0;

    // skip the line offset
    while (is && line_num < line_offset)
    {
        std::string s_line;
        if (std::getline(is, s_line))
            ++line_num;
    } // end while

    // read samples until we reach number of samples requested or end of file
    while (is && samples_read < num_samples)
    {
        std::string s_line;
        if (std::getline(is, s_line))
        {
            ++line_num;
            if (!EDLHelper::isCommentOrEmpty(s_line))
            {
                auto csv_sample_tokens = hebench::Utilities::CSVTokenizer::tokenizeLine(s_line);
                if (csv_sample_tokens.size() < EDLHelper::CSVLine::CSVTokenSize)
                {
                    std::stringstream ss;
                    ss << "Invalid CSV sample detected. A CSV sample must follow the following format: <filename>[, <from_line>[, <num_samples>]]";
                    throw std::runtime_error(ss.str());
                } // end if

                std::uint64_t csv_line_offset  = 0;
                std::uint64_t csv_num_samples  = std::numeric_limits<std::uint64_t>::max();
                std::filesystem::path csv_path = csv_sample_tokens[EDLHelper::CSVLine::Index_CSVFilename];
                if (csv_path.is_relative())
                    csv_path = this->m_working_path / csv_path;
                csv_path = std::filesystem::canonical(csv_path);

                std::ifstream fnum_csv;
                err_msg.clear();
                try
                {
                    fnum_csv.open(csv_path, std::ifstream::in);
                    if (!fnum_csv.is_open())
                        throw std::ios_base::failure("Unable to open file for reading: " + std::string(csv_path));
                }
                catch (std::exception &ex)
                {
                    err_msg = std::string(": ") + ex.what();
                }
                catch (...)
                {
                    err_msg = ".";
                }
                if (!err_msg.empty())
                {
                    std::stringstream ss;
                    ss << "Error occurred attempting to parse CSV sample" << err_msg;
                    throw std::runtime_error(ss.str());
                } // end if

                if (csv_sample_tokens.size() > EDLHelper::CSVLine::Index_CSVFromLine)
                {
                    // read custom batch sizes
                    err_msg.clear();
                    try
                    {
                        const std::string &s_csv_line_offset = csv_sample_tokens[EDLHelper::CSVLine::Index_CSVFromLine];
                        std::stringstream ss                 = std::stringstream(s_csv_line_offset);
                        if (!(ss >> csv_line_offset))
                            throw std::runtime_error("Invalid value \"" + s_csv_line_offset + "\".");
                    }
                    catch (std::exception &ex)
                    {
                        err_msg = std::string(": ") + ex.what();
                    }
                    catch (...)
                    {
                        err_msg = ".";
                    }
                    if (!err_msg.empty())
                    {
                        std::stringstream ss;
                        ss << "Error reading <from_line> from CSV sample" << err_msg;
                        throw std::runtime_error(ss.str());
                    } // end if
                } // end if

                if (csv_sample_tokens.size() > EDLHelper::CSVLine::Index_CSVNumSamples)
                {
                    err_msg.clear();
                    try
                    {
                        const std::string &s_csv_num_samples = csv_sample_tokens[EDLHelper::CSVLine::Index_CSVNumSamples];
                        std::stringstream ss                 = std::stringstream(s_csv_num_samples);
                        if (!(ss >> csv_num_samples))
                            throw std::runtime_error("Invalid value \"" + s_csv_num_samples + "\".");
                    }
                    catch (std::exception &ex)
                    {
                        err_msg = std::string(": ") + ex.what();
                    }
                    catch (...)
                    {
                        err_msg = ".";
                    }
                    if (!err_msg.empty())
                    {
                        std::stringstream ss;
                        ss << "Error reading <num_samples> from CSV sample" << err_msg;
                        throw std::runtime_error(ss.str());
                    } // end if
                } // end if

                // CSV file specified in line successfully opened.
                // Treat contents of CSV file as local data block (with header already parsed).
                EDLTypedHelper<T> helper(m_pad_to_length, m_pad_value, csv_path.parent_path());
                std::uint64_t csv_line_num = 0;
                helper.readLocalDataBlock(out_data,
                                          fnum_csv,
                                          csv_line_offset, csv_num_samples,
                                          csv_line_num);

                // CSV sample line read
                ++samples_read;
            } // end if
        } // end if
    } // end while
}

template <typename T>
std::vector<T> EDLTypedHelper<T>::parseDataSample(std::string_view s_csv_line_sample)
{
    std::vector<T> retval;
    auto csv_tokens = hebench::Utilities::CSVTokenizer::tokenizeLine(s_csv_line_sample);

    std::uint64_t size_since_last_pad = 0;
    for (std::size_t i = 0; i < csv_tokens.size(); ++i)
    {
        if (csv_tokens[i].empty())
        {
            // add padding as requested
            if (m_pad_to_length > 0)
                // if pad requested and size has been satisfied, no padding is added
                while (size_since_last_pad < m_pad_to_length)
                {
                    retval.emplace_back(m_pad_value);
                    ++size_since_last_pad;
                } // end while
            else
                // if no pad requested, empty values are set to the pad value
                retval.emplace_back(m_pad_value);
            // reset pad to start a new section
            size_since_last_pad = 0;
        } // end if
        else
        {
            // read the next value
            std::vector<T> value(1);
            std::stringstream ss = std::stringstream(csv_tokens[i]);
            try
            {
                if (!(ss >> value.front()))
                    throw std::runtime_error("Invalid value.");
            }
            catch (...)
            {
                // error reading value, attempt to read it as a string
                value = EDLTypedHelper<T>::parseTokenAsString(csv_tokens[i]);
            }

            retval.insert(retval.end(), value.begin(), value.end());
            size_since_last_pad += value.size();
        } // end else
    } // end for

    // pad if we still have leftover space
    if (m_pad_to_length > 0)
        while (size_since_last_pad < m_pad_to_length)
        {
            retval.emplace_back(m_pad_value);
            ++size_since_last_pad;
        } // end while

    return retval;
}

template <typename T>
std::vector<T> EDLTypedHelper<T>::parseTokenAsString(std::string_view sv_token)
{
    std::vector<T> retval;

    hebench::Utilities::trim(sv_token);

    if (!sv_token.empty())
    {
        // remove surrounding quotations, if any
        if (sv_token.front() == '\"')
            sv_token.remove_prefix(1);
        if (sv_token.back() == '\"')
            sv_token.remove_suffix(1);

        retval.reserve(sv_token.length());
        for (auto it = sv_token.begin(); it != sv_token.end(); ++it)
            retval.emplace_back(static_cast<T>(*it));
    } // end if

    return retval;
}

//------------------------------
// class ExternalDatasetLoader
//------------------------------

template <typename T, typename E>
ExternalDataset<T> ExternalDatasetLoader<T, E>::loadFromCSV(const std::string &filename,
                                                            std::uint64_t max_loaded_size)
{
    std::uint64_t line_num = 0;
    ExternalDataset<T> retval;

    std::ifstream fnum;

    std::filesystem::path filepath = std::filesystem::canonical(filename);

    fnum.open(filepath, std::ifstream::in);
    if (!fnum.is_open())
        throw std::ios_base::failure("Unable to open file for reading: " + std::string(filepath));

    //EDLTypedHelper<T> helper(pad_to_length, pad_value, filename);

    std::string err_msg;
    try
    {

        while (fnum)
        {
            std::string s_line;
            if (std::getline(fnum, s_line))
            {
                ++line_num; // new line
                if (!EDLHelper::isCommentOrEmpty(s_line))
                {
                    // read control line
                    auto csv_tokens = hebench::Utilities::CSVTokenizer::tokenizeLineInPlace(s_line);
                    if (csv_tokens.size() < EDLHelper::ControlLine::ControlTokenSize)
                    {
                        std::stringstream ss;
                        ss << "ExternalDatasetLoader: in file " << filename << ":" << line_num << ": Invalid number of items in control line. Expected "
                           << EDLHelper::ControlLine::ControlTokenSize << ", but " << csv_tokens.size() << " found.";
                        throw std::runtime_error(ss.str());
                    } // end if
                    std::vector<std::vector<std::vector<T>>> *data = nullptr;
                    if (csv_tokens[EDLHelper::ControlLine::Index_ControlIdentifier] == EDLHelper::ControlLineInput)
                        data = &retval.inputs;
                    else if (csv_tokens[EDLHelper::ControlLine::Index_ControlIdentifier] == EDLHelper::ControlLineOutput)
                        data = &retval.outputs;
                    else
                    {
                        std::stringstream ss;
                        ss << "ExternalDatasetLoader: in file " << filename << ":" << line_num << ": Invalid control line identifier: \""
                           << csv_tokens[EDLHelper::ControlLine::Index_ControlIdentifier] << "\".";
                        throw std::runtime_error(ss.str());
                    } // end else
                    EDLTypedHelper<T>::readDataBlock(*data,
                                                     fnum,
                                                     csv_tokens,
                                                     filepath.parent_path(),
                                                     line_num);
                } // end if
            } // end if
        } // end while
    }
    catch (std::exception &ex)
    {
        err_msg = std::string(": ") + ex.what();
    }
    catch (...)
    {
        err_msg = std::string(" line.");
    }
    if (!err_msg.empty())
    {
        std::stringstream ss;
        ss << "ExternalDatasetLoader: in file " << filepath << ":" << line_num
           << ": Error occurred while parsing" << err_msg;
        throw std::runtime_error(ss.str());
    } // end if

    fnum.close();

    // validate read size
    std::uint64_t loaded_size = 0;
    if (max_loaded_size > 0)
    {
        for (std::size_t param_i = 0; param_i < retval.inputs.size(); ++param_i)
            for (std::size_t sample_i = 0; sample_i < retval.inputs[param_i].size(); ++sample_i)
                loaded_size += sizeof(T) * retval.inputs[param_i][sample_i].size();
        for (std::size_t result_i = 0; result_i < retval.outputs.size(); ++result_i)
            for (std::size_t sample_i = 0; sample_i < retval.outputs[result_i].size(); ++sample_i)
                loaded_size += sizeof(T) * retval.outputs[result_i][sample_i].size();
    } // end if

    if (loaded_size > max_loaded_size)
    {
        std::stringstream ss;
        ss << "ExternalDatasetLoader: in file " << filepath << ": Loaded data exceeds maximum available size. "
           << "Maximum size expected was " << max_loaded_size << " bytes, but " << loaded_size << " bytes read.";
        throw std::runtime_error(ss.str());
    } // end if

    return retval;
}

} // namespace DataLoader
} // namespace hebench
