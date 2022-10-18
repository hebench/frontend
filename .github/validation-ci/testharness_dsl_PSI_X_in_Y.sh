<<com
PSI sample

Exemplifies one of the main usages of the PSI Workload. Tests the PSI workload for offline and latency by using
all the possible types along with even and odd sizes for 'k'. Also checks the scenario where X is contained in Y.

com

# server paths
CLEARTEXTLIB=$(realpath ./lib/libhebench_cleartext_backend.so)
TESTHARNESS=$(realpath ./bin/test_harness)

# IDs relevant for offline and latency, covering all the types.
for id in 48 49 50 51 52 53 54 55; do
    # testing even and odd k values
    for k in 14 15; do
dataset="input, 0, 1, local, $k, 0
Canada, , United States

input, 1, 1, local, $k, 0
Brazil, , Canada, , Colombia, , Mexico, , United States"

    dataset_file=countries.csv
    echo "$dataset" > "$(pwd)/$dataset_file"


data="default_min_test_time: 0
default_sample_size: 0
random_seed: 1234
benchmark:
  - ID: $id
    dataset: $(pwd)/$dataset_file
    default_min_test_time: 0
    default_sample_sizes:
      0: 0
      1: 0
    params:
      0:
        name: n
        type: UInt64
        value:
          from: 2
          to: 2
          step: 0
      1:
        name: m
        type: UInt64
        value:
          from: 5
          to: 5
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

      "$TESTHARNESS" --backend_lib_path "$CLEARTEXTLIB" --config_file "$(pwd)/$file" | tee test_harness_output.log
      if ! grep -Fxq "[ Info    ] Failed: 0" test_harness_output.log
      then
            echo "Failed to test the PSI's worload with: $0"
            echo "Workload ID: $id"
            echo "k: $k"
            # clean-up
            rm "$(pwd)/$file"
            rm "$(pwd)/$dataset_file"
            exit 1
        else
            echo "Successfully tested Test Harness' PSI with:"
            echo "Workload ID: $id"
            echo "k: $k"
        fi
    done # k related for   
done # ID related for

# clean-up
rm "$(pwd)/$file"
rm "$(pwd)/$dataset_file"

exit 0