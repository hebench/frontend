#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
TMP TEST

Backend should not be located in /tmp

com

BACKEND=./lib/libhebench_example_backend.so
TMPBACKEND=/tmp/libhebench_example_backend.so

cp $BACKEND /tmp
./bin/test_harness --backend_lib_path $TMPBACKEND

status=$?

# Clean up
rm -r $TMPBACKEND

if $status
then
    echo "Tmp Validation Failed"
    exit 1
fi

echo "Tmp Validation PASSED"
exit 0
