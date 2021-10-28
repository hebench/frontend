#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

set -e

echo "This script will install the minimum requirements to run."
echo "It may override previously installed instances of the dependencies therein."
read -r -p "Are you sure you want to continue with this? [y/N] " response
response=${response,,}
if [[ ! "$response" =~ ^(yes|y)$ ]]; then
    echo "User confirmation required for installation."
    echo "Ending script execution now"
    exit 1
fi

echo -e "\nSTARTING REQUIREMENT INSTALLATION...\n"

# Formatting
sudo apt-get update
sudo apt-get -y install python3-pip clang-format-9
pip install pre-commit
pre-commit --version

# CMake
which cmake
mkdir cmake-tmp
pushd cmake-tmp
VERSION=3.15.3 # Use any version 3.13+ (3.15+ required for cmake --install vs make install)
wget https://github.com/Kitware/CMake/releases/download/v$VERSION/cmake-$VERSION-linux-x86_64.sh
sudo chmod +x cmake-$VERSION-linux-x86_64.sh
sudo ./cmake-$VERSION-linux-x86_64.sh --skip-license --prefix=/usr/local # Will overwrite a cmake installation
popd
sudo rm cmake-tmp -rf
echo "Global CMake: $(cmake --version)"
if [ "$(cmake --version | head -n 1)" != "cmake version $VERSION" ]; then 
    echo "Wrong CMake version installed here: "; 
    which cmake; 
    exit 1; 
else 
    echo "Global CMake: $(cmake --version)"; 
fi

# GCC
sudo apt -y install build-essential
sudo apt -y install gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 9
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 9
gcc --version
g++ --version

echo
echo -e "\nCOMPELTED REQUIREMENT INSTALLATION\n"

exit 0

