// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/*
* Copyright (C) 2022 Intel Corporation
* SPDX-License-Identifier: Apache-2.0
*/

#include "hebench_dataset_loader_functional_test.h"

int main(int argc, char **argv)
{
    if (argc < min_args_allowed)
    {
        signal_error("Missing CSV file path.\n", true);
    }
    else if (argc == min_args_allowed)
    {
        if (strcmp(argv[help_arg], help_cstr_short) == 0 || strcmp(argv[help_arg], help_cstr_long) == 0)
        {
            std::cout << help_msg << std::endl;
        }
        else
        {
            signal_error(check_usage);
        }
    }
    else if (argc > max_args_allowed)
    {
        signal_error("The amount of arguments exceed the required.\n", true);
    }
    else
    {
        std::filesystem::path file_path = argv[file_path_arg];
        if (!std::filesystem::exists(file_path))
        {
            signal_error("The provided file does not exist.");
        }
        if (file_path.extension() != ".csv")
        {
            signal_error("The provided file's extension is not CSV related.");
        }
        else
        {
            std::uint64_t max_loaded_size = argc == max_args_allowed ? (std::uint64_t)std::stoul(argv[max_loaded_size_arg], nullptr, 0) : 0;
            std::string data_load_type    = argv[type_arg];
            try
            {
                load_from_csv(data_load_type, file_path, max_loaded_size);
            }
            catch (const std::exception &exc)
            {
                signal_error(exc.what());
            }
        }
    }
    return EXIT_SUCCESS;
}
