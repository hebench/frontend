#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
INVALID BACKEND

Error out when passing in bad .so

com

BADBACKEND=bad-be.so
echo "BAD INPUT" >> $BADBACKEND
./bin/test_harness --backend_lib_path $BADBACKEND

if $?
then
    echo "Invalid .so validation Failed"
    exit 1
fi

echo "Invalid .so validation PASSED"
exit 0
