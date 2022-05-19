// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "hebench_dataset_loader_functional_test.h"

using namespace hebench::DataLoader;

void signal_error(const char *msg, bool usage)
{
    std::cerr << msg;
    if (usage)
    {
        std::cerr << check_usage;
    }
    std::cerr << std::endl;
    exit(EXIT_FAILURE);
}

std::uint8_t get_type_from_data(std::string data)
{
    std::map<std::string, std::uint8_t> dataTypeMap = { { "i32", i32 }, { "i64", i64 }, { "f32", f32 }, { "f64", f64 } };
    try
    {
        return dataTypeMap.at(data);
    }
    catch (const std::out_of_range &missing_key)
    {
        return data_fail;
    }
}

void load_from_csv(std::string data, std::filesystem::path file_path, std::uint64_t max_loaded_size)
{
    std::uint8_t data_load_type_evaluated = get_type_from_data(data);
    switch (data_load_type_evaluated)
    {
    case i32:
    {
        ExternalDataset<std::int32_t> data_set_i32 =
            ExternalDatasetLoader<std::int32_t>::loadFromCSV(file_path.string(), max_loaded_size);
        break;
    }
    case i64:
    {
        ExternalDataset<std::int64_t> data_set_i64 =
            ExternalDatasetLoader<std::int64_t>::loadFromCSV(file_path.string(), max_loaded_size);
        break;
    }
    case f32:
    {
        ExternalDataset<float> data_set_f32 =
            ExternalDatasetLoader<float>::loadFromCSV(file_path.string(), max_loaded_size);
        break;
    }
    case f64:
    {
        ExternalDataset<double> data_set_f64 =
            ExternalDatasetLoader<double>::loadFromCSV(file_path.string(), max_loaded_size);
        break;
    }
    case data_fail:
    {
        signal_error("Unknown 'data_type' has been specified.");
        break;
    }
    }
}
