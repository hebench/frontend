#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

# Builds HEBench docs.

#exit on any command failure
set -e
tmp_dir=""

error_exit()
{
if [ -d $tmp_dir ]; then
    rm -rf $tmp_dir
fi
if [ "$1" == "0" ]; then
	echo "[ Success ]"
else
	echo "Unexpected error occurred during last operation."
	echo "[  Failed ] Aborted."
fi
}

# call exit function on exit to display success/error message
trap 'error_exit $?' EXIT

pushd .

if [ "${1}" == "-h" ] || [ "${1}" == "--help" ]; then
    echo "HEBench documentation generator."
    echo "  Usage:"
    echo "    script <output_dir> <api_bridge_tag> <path_to_doxygen>"
    echo
    echo "  Positional arguments:"
    echo "    - output_dir: directory where the generated documentation will be stored."
    echo "        Documentation will come already inside a 'docs' directory."
    echo "        Defaults to '..'"
    echo "    - api_bridge_tag: Tag to pull from API Bridge repo. Defaults to 'main'."
    echo "    - path_to_doxygen: Directory where doxygen 1.9.1 is installed."
    echo "        Defaults to empty value (system wide)."
    echo
    exit 0
fi

script_path=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
output_dir=$(realpath "${script_path}/../")
doxygen_path=""
doxygen_cmd="doxygen"


echo "Generating documentation"

echo "[    Info ] Checking doxygen version..."

if [ ! -z $3 ]; then
    doxygen_path=$(realpath "${3}")
    if [ ! -d $doxygen_path ]; then
        echo "[   Error ] Invalid path to doxygen. Path does not exists:"
        echo "            ${doxygen_path}"
        exit 1
    fi
    doxygen_cmd=$doxygen_path/doxygen
fi
if [ "${doxygen_path}" == "" ]; then
    echo "[    Info ] Doxygen location: (system wide)"
else
    echo "[    Info ] Doxygen location: ${doxygen_path}"
fi

set +e
doxy_version_cmd=(${doxygen_cmd} -v)
doxy_version=$(${doxy_version_cmd[@]})
retval=$?
set -e
if [ $retval -ne 0 ] || [ $doxy_version != "1.9.1" ]; then
    echo "Required doxygen v1.9.1 not found in"
    if [ "${doxygen_path}" == "" ]; then
        echo "(system path)"
    else
        echo "${doxygen_path}"
    fi
    read -p "Would you like to install it SYSTEM WIDE (requires admin)? (Y/n) " -r
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        chmod +x $script_path/doxygen_1.9.1_install.sh
        $script_path/doxygen_1.9.1_install.sh
    fi
    # check installation succeeded
    doxygen_cmd=doxygen
    doxy_version_cmd=(${doxygen_cmd} -v)
    set +e
    doxy_version=$(${doxy_version_cmd[@]})
    retval=$?
    set -e
    if [ $retval -ne 0 ] || [ $doxy_version != "1.9.1" ]; then
        echo "[   Error ] Required doxygen v1.9.1 not found."
        exit 1
    fi
fi

if [ ! -z $1 ]; then
    output_dir=$(realpath "${1}")
fi
if [ ! -d $output_dir ]; then
    echo "[   Error ] Invalid output path. Path does not exists:"
    echo "            ${output_dir}"
    exit 1
fi
echo "[    Info ] Output location: ${output_dir}"

tag_version="main"
if [ ! -z $2 ]; then
    tag_version=$2
fi
echo "[    Info ] API Bridge tag: ${tag_version}"
echo
echo "[    Info ] Creating temporary working location..."
tmp_dir=$(mktemp -d)
cd $tmp_dir

# prepare the doc sources

echo "[    Info ] Cloning API Bridge project..."
pushd .
git clone https://github.com/hebench/api-bridge.git api_bridge
if [ ! -d "${tmp_dir}/api_bridge" ]; then
    echo "[   Error ] Failed cloning API Bridge project."
    exit 1
fi
cd ${tmp_dir}/api_bridge
git checkout ${tag_version}
popd
echo "[    Info ] Copying documentation source files..."
cp -R $script_path/../test_harness $tmp_dir
cp -R $script_path/../docsrc $tmp_dir

echo "[    Info ] Configuring doxygen..."
mv $tmp_dir/docsrc/DoxygenLayout.xml.in $tmp_dir/DoxygenLayout.xml
mv $tmp_dir/docsrc/doxyfile.in $tmp_dir/doxyfile

tmp_sed_text="s;@API_BRIDGE_LOCATION;./api_bridge;g"
sed -i "${tmp_sed_text}" $tmp_dir/doxyfile
tmp_sed_text="s;@OUTPUT_DIR;${output_dir};g"
sed -i "${tmp_sed_text}" $tmp_dir/doxyfile

echo "[    Info ] Generating documentation..."
${doxygen_cmd}

popd
echo "[    Info ] Clean up..."
rm -rf $tmp_dir

echo
echo "[    Info ] API Bridge tag used: ${tag_version}"
echo "[    Info ] Documentation saved to:"
echo "            ${output_dir}"
echo

exit 0

