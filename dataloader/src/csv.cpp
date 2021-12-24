// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "interface.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

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
        imbue(loc);
    }
};

std::locale icsvstream::loc = std::locale(std::locale(), new icsvstream::csv_ws());
;

template <typename T>
static void loadcsvdatafile(std::ifstream &ifs, std::vector<std::vector<T>> &v, size_t nlines, size_t skip = 0)
{
    std::string line;
    while (skip--)
        std::getline(ifs, line);
    while (nlines-- and std::getline(ifs, line))
    {
        icsvstream ils(line);
        std::vector<T> w;
        std::istream_iterator<T> isit(ils);
        std::copy(isit, std::istream_iterator<T>(), std::back_inserter(w));
        v.push_back(w);
    }
}

template <typename T, typename E>
ExternalDataset<T> ExternalDatasetLoader<T, E>::loadFromCSV(const std::string &filename, std::uint64_t max_loaded_size)
{
    ExternalDataset<T> eds;
    std::ifstream ifs(filename, std::ifstream::in);
    std::string metaline;
    while (std::getline(ifs, metaline))
    {
        if (metaline.at(0) == '#' or metaline.size() == 0)
            continue;
        icsvstream iss(metaline);
        std::string tag, kind;
        size_t ix, nlines;
        iss >> tag >> ix >> nlines >> kind;
        std::vector<std::vector<std::vector<T>>> *u;
        if (tag == "input")
            u = &eds.inputs;
        if (tag == "output")
            u = &eds.outputs;
        if (ix > u->size())
            u->resize(ix);
        std::vector<std::vector<T>> &v = u->at(ix - 1);
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
                std::string path = filename.substr(0, filename.find_last_of("/\\") + 1) + fname;
                std::ifstream ifs_csv(path, std::ifstream::in);
                loadcsvdatafile(ifs_csv, v, num_lines, from_line - 1);
                ifs_csv.close();
            }
        }
        if (kind == "local")
        {
            loadcsvdatafile(ifs, v, nlines, 0);
        }
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
