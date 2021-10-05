#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
SYMLINK TEST

Path to config should not be a symlink

com

BACKENDLIB=./lib/libhebench_example_backend.so
CONFIG=./config.yaml
SYMCONFIG=sym_config.yaml

ln -s $CONFIG $SYMCONFIG
./bin/test_harness --config $SYMCONFIG --backend_lib_path $BACKENDLIB

if $?
then
    echo "Symlink Validation Failed"
    exit 1
fi

echo "Symlink Validation PASSED"
exit 0
