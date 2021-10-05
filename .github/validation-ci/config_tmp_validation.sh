#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
TMP TEST

Config should not be located in /tmp

com

BACKEND=./lib/libhebench_example_backend.so
CONFIG=./config.yaml
TMPCONFIG=/tmp/config.yaml

cp $CONFIG /tmp
./bin/test_harness --config_file $TMPCONFIG --backend_lib_path $BACKEND

status=$?

# Clean up
rm -r $TMPCONFIG

if $status
then
    echo "Tmp Validation Failed"
    exit 1
fi

echo "Tmp Validation PASSED"
exit 0
