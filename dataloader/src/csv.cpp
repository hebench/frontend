// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "interface.h"
#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <typeinfo>

namespace hebench {
namespace dataloader {

struct icsvstream : std::istringstream
{
    struct csv_ws : std::ctype<char>
    {
        static const mask *make_table()
        {
            static std::vector<mask> v(classic_table(), classic_table() + table_size);
            v[','] |= space;
            //v[' '] &= ~space;
            return &v[0];
        }
        csv_ws(std::size_t refs = 0) :
            ctype(make_table(), false, refs) {}
    };
    static std::locale loc;
    template <class... Args>
    icsvstream(Args &&... args) :
        std::istringstream(std::forward<Args>(args)...)
    {
        exceptions(std::ifstream::failbit);
        imbue(loc);
    }
};

std::locale icsvstream::loc = std::locale(std::locale(), new icsvstream::csv_ws());

template <typename T>
static void loadcsvdatafile(std::ifstream &ifs, std::vector<std::vector<T>> &v, size_t nlines, size_t skip = 0, size_t lnum = 0, std::string fpath = "")
{
    std::string line;
    while (skip--)
    {
        ++lnum;
        std::getline(ifs, line);
    }
    while (nlines-- and std::getline(ifs, line))
    {
        ++lnum;
        std::cerr << "Reading line: " << line << std::endl;
        if (line.size() == 0 or line.at(0) == '#')
        {
            ++nlines;
            continue;
        }
        icsvstream ils(line);
        std::vector<T> w;
        std::istream_iterator<T> isit(ils);
        try
        {
            std::copy(isit, std::istream_iterator<T>(), std::back_inserter(w));
        }
        catch (const std::ios_base::failure &e)
        {
            if (!ils.eof())
            {
                //throw; // preserves error type and attributes
                std::ostringstream oss;
                size_t ofs = ils.rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
                int status;
                oss << e.what() << " in data type \"" << abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) << "\""
                    << " at " << fpath << ":" << lnum << ":" << ofs << std::endl;
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
    }
    if ((nlines + 1) != 0)
    {
        throw(std::length_error("Not enought lines read before end of file."));
    }
}

template <typename T>
icsvstream &operator>>(icsvstream &in, T &arg)
{
    std::istringstream *iss = &in;
    try
    {
        (*iss) >> arg; // superclass read data into argument
    }
    catch (const std::ios_base::failure &e)
    {
        std::ostringstream oss;
        int status;
        oss << e.what() << " in data type \"" << abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) << "\"";
        throw std::ios_base::failure(oss.str(), e.code());
    }
    return in;
}

template <typename T, typename E>
ExternalDataset<T> ExternalDatasetLoader<T, E>::loadFromCSV(const std::string &filename, std::uint64_t max_loaded_size)
{
    size_t lnum = 0;
    ExternalDataset<T> eds;
    std::cerr << "Opening: " << filename << std::endl;
    std::ifstream ifs(filename, std::ifstream::in);
    ifs.exceptions(std::ifstream::badbit);
    std::string metaline;
    while (std::getline(ifs, metaline))
    {
        ++lnum;
        std::cerr << "Reading control line: " << metaline << std::endl;
        if (metaline.size() == 0 or metaline.at(0) == '#')
            continue;
        icsvstream iss(metaline);
        std::string tag, kind;
        size_t ix, nlines;
        iss >> tag >> ix >> nlines >> kind;
        std::vector<std::vector<std::vector<T>>> *u;
        if (tag == "input")
            u = &eds.inputs;
        else if (tag == "output")
            u = &eds.outputs;
        else
            throw std::runtime_error(std::string("parse error: <tag> must be \"input\" or \"output\". Got: ") + tag);
        if (ix >= u->size())
            u->resize(ix + 1);
        std::vector<std::vector<T>> &v = u->at(ix);
        if (kind == "csv")
        {
            while (nlines--)
            {
                std::string dataline;
                std::getline(ifs, dataline);
                icsvstream ils(dataline);
                std::string fname;
                size_t from_line = 1, num_lines = 0;
                ils >> fname >> from_line >> num_lines;
                std::string path(fname);
                if (fname[0] != '/')
                    path = filename.substr(0, filename.find_last_of("/\\") + 1) + fname;
                std::cerr << "Opening data " << path << std::endl;
                std::ifstream ifs_csv(path, std::ifstream::in);
                ifs_csv.exceptions(std::ifstream::badbit);
                loadcsvdatafile(ifs_csv, v, num_lines, from_line - 1, 0, path);
                ifs_csv.close();
            }
        }
        else if (kind == "local")
        {
            std::cerr << "Reading local data" << std::endl;
            loadcsvdatafile(ifs, v, nlines, 0, lnum, filename);
            lnum += nlines;
        }
        else
            throw std::runtime_error(std::string("parse error: <kind> must be \"local\" or \"csv\". Got: ") + kind);
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
    return eds;
}

template <typename T, typename E>
void ExternalDatasetLoader<T, E>::exportToCSV(const std::string &filename, const ExternalDataset<T> &dataset)
{
    // DO NOT IMPLEMENT
    throw std::runtime_error("Not implemented");
}
} // namespace dataloader
} // namespace hebench
