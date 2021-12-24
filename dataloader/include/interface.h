// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace hebench {
namespace dataloader {

template <typename T>
struct ExternalDataset
{
    std::vector<std::vector<std::vector<T>>> inputs;
    std::vector<std::vector<std::vector<T>>> outputs;
};

template <typename T,
          // Only `int32_t`, `int64_t`, `float`, and `double` are supported as types.
          typename = std::enable_if_t<
              std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::int64_t> || std::is_same_v<T, float> || std::is_same_v<T, double>>>
class ExternalDatasetLoader
{
public:
    ExternalDataset<T> loadFromCSV(const std::string &filename, std::uint64_t max_loaded_size = 0);
    void exportToCSV(const std::string &filename, const ExternalDataset<T> &dataset);
};

template class ExternalDatasetLoader<int32_t>;
template class ExternalDatasetLoader<int64_t>;
template class ExternalDatasetLoader<float>;
template class ExternalDatasetLoader<double>;

template class ExternalDataset<int32_t>;
template class ExternalDataset<int64_t>;
template class ExternalDataset<float>;
template class ExternalDataset<double>;

} // namespace dataloader
} // namespace hebench
