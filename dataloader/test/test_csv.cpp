// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "interface.h"
#include <filesystem>
#include <iostream>

using namespace hebench::dataloader;

int main(int, char **)
{
    std::cout << "Hello, test!\n";
    { //anonymous testing scope
        ExternalDatasetLoader<int> dl;
        auto ds = dl.loadFromCSV("data/input_int.csv");
        if (
            !(ds.inputs[0][0].size() == 16
              && ds.inputs[1][0].size() == 1
              && ds.inputs[2].size() == 5
              && ds.inputs[2][0].size() == 16
              && ds.inputs[2][0][0] == -15
              && ds.outputs[0].size() == 5
              && ds.outputs[0][4][0] == 4))
            exit(-1);
    }
    {
        ExternalDatasetLoader<float> dl;
        auto ds = dl.loadFromCSV("data/input_float.csv");
        if (
            !(ds.inputs[0][0].size() == 16
              && ds.inputs[1][0].size() == 1
              && ds.inputs[2].size() == 5
              && ds.inputs[2][0].size() == 16
              && ds.inputs[2][0][0] == -15
              && ds.outputs[0].size() == 5
              && ds.outputs[0][0][0] == std::stof("0.000911051")))
            exit(-1);
    }
    {
        ExternalDatasetLoader<float> dl;
        try
        {
            auto ds = dl.loadFromCSV("data/input_bad.csv");
            exit(-1); //should not get to here
        }
        catch (std::ios_base::failure &e)
        {
            std::cerr << "caught: " << e.what() << std::endl;
        }
    }
    {
        for (auto &f : std::filesystem::directory_iterator("data/correct"))
        {
            if (f.path().compare("data/correct/correct") < 0)
            {
                std::cerr << "Reading: " << f.path() << std::endl;
                ExternalDatasetLoader<float> dl;
                auto ds = dl.loadFromCSV(f.path());
            }
        }
        for (auto &f : std::filesystem::directory_iterator("data/bad"))
        {
            std::cerr << "Reading: " << f.path() << std::endl;
            ExternalDatasetLoader<float> dl;
            try
            {
                auto ds = dl.loadFromCSV(f.path());
                exit(-1); // should throw error before here
            }
            catch (std::exception &e)
            {
                std::cerr << "caught: " << e.what() << std::endl;
            }
        }
    }
    exit(0);
}
