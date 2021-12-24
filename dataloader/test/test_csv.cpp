// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "interface.h"
#include <iostream>

using namespace hebench::dataloader;

int main(int, char **)
{
    std::cout << "Hello, test!\n";
    { //anonymous testing scope
        ExternalDatasetLoader<int> dl;
        auto ds = dl.loadFromCSV("data/input.csv");
        if (
            !(ds.inputs[0][0].size() == 16
              && ds.inputs[1][0].size() == 1
              && ds.inputs[2].size() == 5
              && ds.inputs[2][0].size() == 16
              && ds.inputs[2][0][0] == -15
              && ds.outputs[0].size() == 5
              && ds.outputs[0][0][0] == 0))
            return -1;
    }
    {
        ExternalDatasetLoader<float> dl;
        auto ds = dl.loadFromCSV("data/input.csv");
        if (
            !(ds.inputs[0][0].size() == 16
              && ds.inputs[1][0].size() == 1
              && ds.inputs[2].size() == 5
              && ds.inputs[2][0].size() == 16
              && ds.inputs[2][0][0] == -15
              && ds.outputs[0].size() == 5
              && ds.outputs[0][0][0] == std::stof("0.000911051")))
            return -1;
    }
    return 0;
}
