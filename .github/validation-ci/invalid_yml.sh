#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
INVALID YAML

Error out when passing in bad yaml

com

BACKEND=./lib/libhebench_example_backend.so
BADCONFIG=bad-config.yaml

sed "1 a BAD INPUT" config.yaml >> $BADCONFIG
./bin/test_harness --config_file $BADCONFIG --backend_lib_path $BACKEND

if $?
then
    echo "Invalid yaml validation Failed"
    exit 1
fi

echo "Invalid yaml validation PASSED"
exit 0
