#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
Negative cases for the Dataset Loader

Testing scenario for unexistent files
com

DATA_LOADER=../dataset_loader/tests/build/bin/hebench_dataset_loader_test
RESULT=0

data="input, 0, 3, local
1,2,3,4
4,5,6,7
8, 9, 10, 11
input,1,1,csv
# cannot open file: file not found or no permission
nofile.csv, 3,1
output, 0, 6, local
1, 2, 3
4, 5, 6
7, 8, 9
10, 20, 30
40, 50, 60
70, 80, 90"

file=test_file.csv

echo "$data" > "$file"

for data_type in i32 i64 f32 f64; do
    "$DATA_LOADER" "$file" "$data_type"
    if [ $? -eq 0 ]; then
        echo "Failed to run the dataset_loader with $file and $data_type"
        RESULT=1
    fi
done 

rm "$file"

exit $RESULT
