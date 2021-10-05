#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

set -e

REPODIR=$PWD

echo "[ KW SETUP SCRIPT ] Setting up Repo for Klocwork Analysis..."


TMP_REPO_DIR=$(mktemp -d)
API_BRIDGE_DIR=api-bridge
YAML_CPP_DIR=yaml-cpp

echo "[ KW SETUP SCRIPT ] Created temporary directory: $TMP_REPO_DIR"

pushd $TMP_REPO_DIR
git clone -b development https://github.com/hebench/api-bridge.git $API_BRIDGE_DIR
git clone -b yaml-cpp-0.6.3 https://github.com/jbeder/yaml-cpp $YAML_CPP_DIR

pushd $API_BRIDGE_DIR
mkdir -p build && cd build && mkdir install
cmake -DCMAKE_INSTALL_PREFIX=./install ..
make -j install
popd

pushd $YAML_CPP_DIR
mkdir -p build && cd build && mkdir install
cmake -DCMAKE_INSTALL_PREFIX=./install -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_INSTALL=ON ..
make -j install
popd

popd

mkdir -p build && pushd build
cmake -DCMAKE_INSTALL_PREFIX=./install -DAPI_BRIDGE_INSTALL_DIR=$TMP_REPO_DIR/$API_BRIDGE_DIR/build/install -DYAML_CPP_INSTALL_DIR=$TMP_REPO_DIR/$YAML_CPP_DIR/build/install ..
popd

echo "[ KW SETUP SCRIPT ] Setting variable to delete tmp directory (CLEANUP_TEMP_REPO_DIR): $TMP_REPO_DIR"
echo "CLEANUP_TEMP_REPO_DIR=$(echo $TMP_REPO_DIR)" >> $GITHUB_ENV

echo "[ KW SETUP SCRIPT ] Finished setting up Repo for Klocwork Analysis"
