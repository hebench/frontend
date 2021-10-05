#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
SYMLINK TEST

Path to backend should not be a symlink

com

BACKENDLIB=./lib/libhebench_example_backend.so
SYMBACKEND=sym_backend.so

ln -s $BACKENDLIB $SYMBACKEND
./bin/test_harness --backend_lib_path $SYMBACKEND

if $?
then
    echo "Symlink Validation Failed"
    exit 1
fi

echo "Symlink Validation PASSED"
exit 0
