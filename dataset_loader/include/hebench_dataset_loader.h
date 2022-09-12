
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DatasetLoader_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DatasetLoader_H_0596d40a3cce4b108a81595c50eb286d

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace hebench {
namespace DataLoader {

template <typename T>
struct ExternalDataset
{
    /**
     * @brief Contains the samples for each input parameter as loaded from external source.
     * @details `inputs[i]` contains all samples for input parameter `i`.
     * `inputs[i][j]` is sample `j` in input parameter `i`. The following must be true:
     * `inputs[i][j0].size() == inputs[i][j1].size()` for all `0 < j0, j1 < inputs[i].size()`.
     * Every sample is a vector of scalars of type `T`.
     */
    std::vector<std::vector<std::vector<T>>> inputs;
    /**
     * @brief Contains the samples for each result component as loaded from external source.
     * @details `outputs[i]` contains all samples for output component `i`.
     * `outputs[i][j]` is sample `j` in output component `i`. The following must be true:
     * `outputs[i][j0].size() == outputs[i][j1].size()` for all `0 < j0, j1 < outputs[i].size()`.
     * Every sample is a vector of scalars of type `T`.
     */
    std::vector<std::vector<std::vector<T>>> outputs;
};

template <typename T,
          // Only `int32_t`, `int64_t`, `float`, and `double` are supported as types.
          typename = std::enable_if_t<std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::int64_t> || std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, std::string>>>
class ExternalDatasetLoader
{
public:
    /**
     * @brief Loads a dataset from an external csv file that follows the defined structure.
     * @param[in] filename Name of the file to load.
     * @param[in] max_loaded_size Maximum size, in bytes, allowed for the data to occupy after
     * loading. If 0, there is no limit to how much memory the data can occupy.
     * @throws Standard C++ exception derived from std::exception on error describing the
     * failure.
     * @return An `ExternalDataset` structure containing the data loaded where the loaded scalars
     * are of type `T`.
     * @details Contents of the file loaded will be casted to the type indicated for the template
     * parameter specification during the method call.
     *
     * If \p max_loaded_size is not zero, then, the total number of bytes required to hold all
     * the values loaded in the returned `ExternalDataset` must be less than or equal to value of
     * \p max_loaded_size . Otherwise, an exception is thrown because the size of the dataset in
     * the file specified after loading and converting to `ExternalDataset` is larger than
     * non-zero \p max_loaded_size .
     */
    static ExternalDataset<T> loadFromCSV(const std::string &filename,
                                          std::uint64_t max_loaded_size = 0);
};

// template implementations

template class ExternalDatasetLoader<int32_t>;
template class ExternalDatasetLoader<int64_t>;
template class ExternalDatasetLoader<float>;
template class ExternalDatasetLoader<double>;
template class ExternalDatasetLoader<std::string>;

} // namespace DataLoader
} // namespace hebench

#endif // defined _HEBench_Harness_DatasetLoader_H_0596d40a3cce4b108a81595c50eb286d
