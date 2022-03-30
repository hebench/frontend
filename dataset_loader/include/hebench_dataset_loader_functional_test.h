// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/*
* Copyright (C) 2022 Intel Corporation
* SPDX-License-Identifier: Apache-2.0
*/

#ifndef __HEBENCH_DATASET_LOADER_FUNCTIONAL_TEST__H__
#define __HEBENCH_DATASET_LOADER_FUNCTIONAL_TEST__H__

#include "hebench_dataset_loader.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>

enum DataType
{
    i32,
    i64,
    f32,
    f64,
    data_fail
};

constexpr std::uint8_t help_arg = 1;
constexpr const char *help_msg  = "NAME\n\tdata_loader - launches the data loader module.\n\n"
                                 "SYNOPSYS\n\tdata_loader [OPTION] [FILE] [DATA_TYPE] [MAX_LOAD]\n\n"
                                 "DESCRIPTION\n\tIs a launcher app to test the data loader module.\n"
                                 "\tWith no MAX_LOAD, it will be set to 0.\n\n"
                                 "\t-h, --help displays help mesage and exits";
constexpr const char *check_usage = "Please check the correct usage by typing \"data_loader -h\" or"
                                    " \"data_loader --help\".";
constexpr const char *help_cstr_short = "-h";
constexpr const char *help_cstr_long  = "--help";

constexpr std::uint8_t file_path_arg       = 1;
constexpr std::uint8_t type_arg            = 2;
constexpr std::uint8_t max_loaded_size_arg = 3;

constexpr std::uint8_t max_args_allowed = 4;
constexpr std::uint8_t min_args_allowed = 2;

void signal_error(const char *msg, bool usage = false);
std::uint8_t get_type_from_data(std::string);
void load_from_csv(std::string data, std::filesystem::path file_path, std::uint64_t max_loaded_size);

#endif //!__HEBENCH_DATASET_LOADER_FUNCTIONAL_TEST__H__
