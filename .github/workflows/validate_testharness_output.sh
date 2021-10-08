#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

set -e

mkdir validation-test && pushd validation-test
echo "Cloning Latest Cleartext Backend..."
git clone https://github.com/hebench/backend-cpu-cleartext.git
pushd ./backend-cpu-cleartext

mkdir build && pushd build

echo "Configuring & Building Cleartext Backend..."
cmake -DCMAKE_INSTALL_PREFIX=./ -DCMAKE_BUILD_TYPE=Release -DAPI_BRIDGE_INSTALL_DIR=../../../ ..
make -j install

CLEARTEXTLIB=$(realpath ./lib/libhebench_cleartext_backend.so)

popd
popd
popd

pushd bin
./test_harness --config_file ./config.yaml --random_seed 1234 --report_delay 0 --backend_lib_path $CLEARTEXTLIB --dump
sed -i 's/default_min_test_time: 0/default_min_test_time: 50/g' config.yaml
./test_harness --config_file ./config.yaml --random_seed 1234 --report_delay 0 --backend_lib_path $CLEARTEXTLIB |& tee test_harness_output.log

echo "Checking output from Test Harness..."
if ! grep -Fxq "[ Info    ] Failed: 0" test_harness_output.log
then
    echo "Validation Failed"
    exit 1
fi
echo "Validation PASSED"
