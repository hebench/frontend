#!/bin/bash

# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
Positive cases for the Dataset Loader

Testing the correct usage of the Dataset Loader Library
com

DATA_LOADER=/bin/hebench_dataset_loader_test
RESULT=0

data="# elt-wise add
input,0,2, local
5.0,6.0,7.0,8.0,9.0
#comment
-11,10,-9,8,-7
input,1,2,local
-11,10,-9,8,-7
5,6,7,8,9
#input,1,1,csv
#\"in(0,2)/test_file.csv\"
#test_file.csv,1,0
#test_file.csv
# optional: ground truths
# if present, all outputs for operations using all combinations
# of inputs must be given
output,0, 4, local
-6,16,-2,16,2
10,12,14,16,18
-22,20,-18,16,-14
-6,16,-2,16,2"

file=test_file.csv

echo "$data" > "$file"

for data_type in i32 i64 f32 f64; do
    "$DATA_LOADER" "$file" "$data_type"
    if [ $? -eq 1 ]; then
        echo "Failed to run the dataset_loader with $file and $data_type"
        RESULT=1
    fi
done

rm "$file"

exit $RESULT