# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
PSI sample

Exemplifies one of the main usages of the PSI Workload. Tests the PSI workload for offline and latency by using
all the possible types along with even and odd sizes for 'k'. In this case, the k from the dataset does not
match the values in the config file.
com

# server paths
CLEARTEXTLIB=$(realpath ./lib/libhebench_cleartext_backend.so)
TESTHARNESS=$(realpath ./bin/test_harness)

dataset="input, 0, 1, local, 20, 0
Brazil, , Canada, , Colombia, , Mexico, , United States

input, 1, 1, local, 20, 0
Canada, , United States, , Ivory Coast"

dataset_file=countries.csv
echo "$dataset" > "$(pwd)/$dataset_file"

# IDs relevant for offline and latency, covering all the types.
for id in 48 49 50 51 52 53 54 55; do
  # testing k even and odd, 20 does not match with the sequence
  for k in 14 15; do

data="default_min_test_time: 0
default_sample_size: 0
random_seed: 1234
benchmark:
  - ID: $id
    dataset: $(pwd)/$dataset_file
    default_min_test_time: 0
    default_sample_sizes:
      0: 1
      1: 1
    params:
      0:
        name: n
        type: UInt64
        value:
          from: 5
          to: 5
          step: 0
      1:
        name: m
        type: UInt64
        value:
          from: 3
          to: 3
          step: 0
      2:
        name: k
        type: UInt64
        value:
          from: $k
          to: $k
          step: 0"

      file=psi.yaml
      echo "$data" > "$(pwd)/$file"

      "$TESTHARNESS" --backend_lib_path "$CLEARTEXTLIB" --config_file "$(pwd)/$file"
      if [ $? -ne 255 ];
      then
          echo "Failed to test the PSI's worload with: $0"
          echo "Workload ID: $id"
          echo "k: $k"
          # clean-up
          rm "$(pwd)/$file"
          rm "$(pwd)/$dataset_file"
          exit 1
      else
          echo "Successfully tested Test Harness' PSI with: $0"
          echo "Workload ID: $id"
          echo "k: $k"
      fi
    done # k related for  
done # ID related for

# clean-up
rm "$(pwd)/$file"
rm "$(pwd)/$dataset_file"

exit 0
