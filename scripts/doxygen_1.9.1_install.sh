#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

set -e

pushd .
tmp_dir=$(mktemp -d)
cd $tmp_dir
wget https://github.com/doxygen/doxygen/archive/Release_1_9_1.tar.gz
tar xvf Release_1_9_1.tar.gz
cd doxygen-Release_1_9_1/
mkdir build
cd build
sudo apt-get update
yes | sudo apt-get install graphviz
yes | sudo apt-get install flex
yes | sudo apt-get install bison
cmake ..
make -j 4
sudo make install -j 4
doxygen -v
popd
rm -rf $tmp_dir
exit 0

