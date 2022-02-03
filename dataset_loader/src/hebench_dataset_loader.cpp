// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cxxabi.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <typeinfo>

#include "hebench_dataset_loader.h"

namespace hebench {
namespace DataLoader {

struct icsvstream : std::istringstream
{
    static constexpr const char *s_trim = " \t\n\r\f\v";

    template <class... Args>
    icsvstream(Args &&... args) :
        std::istringstream(std::forward<Args>(args)...)
    {
        exceptions(std::ifstream::failbit);
    }
};

template <typename T>
icsvstream &operator>>(icsvstream &in, T &arg)
{
    std::string s;
    std::getline(in, s, ',');
    std::istringstream iss(s);
    iss.exceptions(std::ifstream::failbit);
    try
    {
        iss >> arg;
    }
    catch (const std::ios_base::failure &e)
    {
        std::ostringstream oss;
        int status;
        oss << e.what() << " in data type \"" << abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) << "\" from \"" << s << "\"";
        throw std::ios_base::failure(oss.str(), e.code());
    }
    return in;
}

template <>
icsvstream &operator>><std::string>(icsvstream &in, std::string &arg)
{
    char c;
    while (std::isspace(c = in.get()))
        ;
    if (c == '"')
    {
        std::getline(in, arg, '"');
        try
        {
            std::string s;
            std::getline(in, s, ',');
        }
        catch (const std::ios_base::failure &ex)
        {
            if (!in.eof())
                throw;
        }
    }
    else
    {
        in.unget();
        std::getline(in, arg, ',');
        arg.erase(arg.find_last_not_of(" \n\r\t") + 1);
    }
    return in;
}

template <typename T>
static void loadcsvdatafile(std::ifstream &ifs, size_t &lnum, std::vector<std::vector<T>> &v, size_t nlines, size_t skip = 0, std::string fpath = "")
{
    std::string line;
    while (skip--)
    {
        ++lnum;
        std::getline(ifs, line);
    }
    while (std::getline(ifs, line))
    {
        ++lnum;
        line.erase(0, line.find_first_not_of(icsvstream::s_trim));
        line.erase(line.find_last_not_of(icsvstream::s_trim) + 1);
        if (line.size() == 0 or line.at(0) == '#')
            continue;
        icsvstream ils(line);
        std::vector<T> w;
        try
        {
            T x;
            while (!ils.eof() and ils >> x)
            {
                w.push_back(x);
            }
        }
        catch (const std::ios_base::failure &e)
        {
            if (!ils.eof())
            {
                //throw; // preserves error type and attributes
                std::ostringstream oss;
                size_t ofs = ils.rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
                oss << e.what() << " at " << fpath << ":" << lnum << ":" << ofs << std::endl;
                throw std::ios_base::failure(oss.str(), e.code());
            }
        }
        if (v.size() and w.size() != v.back().size())
        {
            std::ostringstream ss;
            ss << "Inconsistent number of values read from line " << lnum << ". Previous: " << v.back().size() << ". This: " << w.size();
            throw std::length_error(ss.str());
        }
        v.push_back(w);
        if (nlines and --nlines == 0)
            break;
    }
    if (nlines)
    {
        throw(std::length_error("Not enought lines read before end of file in " + fpath));
    }
}

template <typename T, typename E>
ExternalDataset<T> ExternalDatasetLoader<T, E>::loadFromCSV(const std::string &filename, std::uint64_t max_loaded_size)
{
    size_t lnum = 0;
    ExternalDataset<T> eds;
    std::ifstream ifs(filename, std::ifstream::in);
    ifs.exceptions(std::ifstream::badbit);
    std::string metaline;
    while (std::getline(ifs, metaline))
    {
        ++lnum;
        metaline.erase(0, metaline.find_first_not_of(icsvstream::s_trim));
        metaline.erase(metaline.find_last_not_of(icsvstream::s_trim) + 1);
        if (metaline.size() == 0 or metaline.at(0) == '#')
            continue;
        icsvstream iss(metaline);
        std::string tag, kind;
        size_t ix, nlines;
        try
        {
            iss >> tag >> ix >> nlines >> kind;
        }
        catch (const std::ios_base::failure &e)
        {
            //throw; // preserves error type and attributes
            std::ostringstream oss;
            size_t ofs = iss.rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
            oss << e.what() << " at " << filename << ":" << lnum << ":" << ofs << std::endl;
            throw std::ios_base::failure(oss.str(), e.code());
        }
        std::vector<std::vector<std::vector<T>>> *u;
        if (tag == "input")
            u = &eds.inputs;
        else if (tag == "output")
            u = &eds.outputs;
        else
        {
            std::ostringstream ss;
            ss << "Parse error at " << filename << ":" << lnum
               << ": <tag> must be \"input\" or \"output\". Got: " << tag;
            throw std::runtime_error(ss.str());
        }
        if (ix >= u->size())
            u->resize(ix + 1);
        std::vector<std::vector<T>> &v = u->at(ix);
        if (kind == "csv")
        {
            while (nlines--)
            {
                std::string dataline;
                std::getline(ifs, dataline);
                dataline.erase(0, dataline.find_first_not_of(icsvstream::s_trim));
                dataline.erase(dataline.find_last_not_of(icsvstream::s_trim) + 1);
                if (dataline.size() == 0 or dataline.at(0) == '#')
                {
                    ++nlines;
                    continue;
                }
                icsvstream ils(dataline);
                std::string fname;
                size_t from_line = 1, num_lines = 0;
                ils >> fname;
                try
                {
                    ils >> from_line >> num_lines;
                }
                catch (const std::ios_base::failure &e)
                {
                    if (!ils.eof())
                        throw;
                }
                std::filesystem::path path = fname;
                if (path.is_relative())
                    path = std::filesystem::path(filename).remove_filename() / path;
                path = std::filesystem::canonical(path);
                std::ifstream ifs_csv(path, std::ifstream::in);
                ifs_csv.exceptions(std::ifstream::badbit);
                std::size_t tmp_lnum = 0;
                loadcsvdatafile(ifs_csv, tmp_lnum, v, num_lines, from_line - 1, path);
                ifs_csv.close();
            }
        }
        else if (kind == "local")
        {
            loadcsvdatafile(ifs, lnum, v, nlines, 0, filename);
        }
        else
        {
            std::ostringstream ss;
            ss << "Parse error at " << filename << ":" << lnum
               << ": <kind> must be \"local\" or \"csv\". Got: " << kind;
            throw std::runtime_error(ss.str());
        }
    }
    if (eds.outputs.size())
    {
        size_t n = 1;
        for (auto &v : eds.inputs)
            n *= v.size();
        if (eds.outputs[0].size() != n)
            throw std::length_error("Output size (" + std::to_string(eds.outputs[0].size()) + ") must be the product of the input sizes (" + std::to_string(n) + ").");
    }
    ifs.close();

    // make sure we did not go above permited max size
    if (max_loaded_size > 0)
    {
        std::uint64_t loaded_size = 0;
        for (const auto &input_component : eds.inputs)
            for (const auto &input_sample : input_component)
                loaded_size += (input_sample.size() * sizeof(T));
        for (const auto &output_component : eds.outputs)
            for (const auto &output_sample : output_component)
                loaded_size += (output_sample.size() * sizeof(T));
        if (loaded_size > max_loaded_size)
            throw std::runtime_error("Loaded size greater than maximum permitted. Maximum: "
                                     + std::to_string(max_loaded_size) + " bytes; loaded: "
                                     + std::to_string(loaded_size) + " bytes.");
    }

    return eds;
}

} // namespace DataLoader
} // namespace hebench
