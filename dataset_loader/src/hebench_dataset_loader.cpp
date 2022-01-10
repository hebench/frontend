// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

#include "hebench_dataset_loader.h"

namespace hebench {
namespace DataLoader {

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
    static constexpr const char *s_trim = " \t\n\r\f\v";
    std::string line;
    while (skip--)
    {
        ++lnum;
        std::getline(ifs, line);
    }
    while (nlines-- and std::getline(ifs, line))
    {
        ++lnum;
        line.erase(0, line.find_first_not_of(s_trim));
        line.erase(line.find_last_not_of(s_trim) + 1);
        std::cerr << "Reading line: " << line << std::endl;
        if (!line.empty() && line.front() != '#') // skip empty lines and comments
        {
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
                    oss << e.what() << " in " << fpath << ":" << lnum << ":" << ofs;
                    throw std::ios_base::failure(oss.str(), e.code());
                }
            }
            if (v.size() and w.size() != v.back().size())
                throw std::length_error("Inconsistent number of values read from line");
            v.push_back(w);
        } // end if
    }
}

template <typename T, typename E>
ExternalDataset<T> ExternalDatasetLoader<T, E>::loadFromCSV(const std::string &filename, std::uint64_t max_loaded_size)
{
    static constexpr const char *s_trim = " \t\n\r\f\v";
    size_t lnum                         = 0;
    ExternalDataset<T> eds;
    std::cerr << "Opening: " << filename << std::endl;
    std::ifstream ifs(filename, std::ifstream::in);
    ifs.exceptions(std::ifstream::badbit);
    std::string metaline;
    while (std::getline(ifs, metaline))
    {
        ++lnum;
        std::cerr << "Reading control line: " << metaline << std::endl;
        metaline.erase(0, metaline.find_first_not_of(s_trim));
        metaline.erase(metaline.find_last_not_of(s_trim) + 1);
        if (metaline.at(0) == '#' or metaline.size() == 0)
            continue;
        icsvstream iss(metaline);
        std::string tag, kind;
        std::size_t ix, nlines;
        iss >> tag >> ix >> nlines >> kind;
        std::vector<std::vector<std::vector<T>>> *u;
        if (tag == "input")
            u = &eds.inputs;
        //else
        if (tag == "output")
            u = &eds.outputs;
        //        else
        //            throw std::runtime_error("Invalid tag in control line: \"" + tag + "\"");
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
        if (kind == "local")
        {
            std::cerr << "Reading local data" << std::endl;
            loadcsvdatafile(ifs, v, nlines, 0, lnum, filename);
            lnum += nlines;
        }
    }
    ifs.close();

    // make sure we did not go above permited max size
    if (max_loaded_size > 0)
    {
        std::uint64_t loaded_size = 0;
        for (std::size_t input_component_i = 0; input_component_i < eds.inputs.size(); ++input_component_i)
            for (std::size_t component_sample_i = 0; component_sample_i < eds.inputs[input_component_i].size(); ++component_sample_i)
                loaded_size += (eds.inputs[input_component_i][component_sample_i].size() * sizeof(T));
        for (std::size_t output_component_i = 0; output_component_i < eds.outputs.size(); ++output_component_i)
            for (std::size_t component_sample_i = 0; component_sample_i < eds.outputs[output_component_i].size(); ++component_sample_i)
                loaded_size += (eds.outputs[output_component_i][component_sample_i].size() * sizeof(T));
        if (loaded_size > max_loaded_size)
            throw std::runtime_error("Loaded size greater than maximum permitted. Maximum: "
                                     + std::to_string(max_loaded_size) + " bytes; loaded: "
                                     + std::to_string(loaded_size) + " bytes.");
    }

    return eds;
}

template <typename T, typename E>
void ExternalDatasetLoader<T, E>::exportToCSV(const std::string &filename, const ExternalDataset<T> &dataset)
{
    (void)filename;
    (void)dataset;
    throw std::runtime_error("Not implemented");
}
} // namespace DataLoader
} // namespace hebench
