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

if [ $# -eq 0 ] || [ "${1}" == "-h" ] || [ "${1}" == "--help" ]; then
    echo "HEBench documentation generator."
    echo "  Usage:"
    echo "    script <output_dir> [<api_bridge_tag> [<path_to_doxygen>]]"
    echo
    echo "  Positional arguments:"
    echo "    - output_dir: directory where the generated documentation will be stored."
    echo "        Documentation will come already inside a 'docs' directory."
    echo "        Default should be '..'"
    echo "    - api_bridge_tag: Tag to pull from API Bridge repo, or local directory"
    echo "        where API Bridge has been cloned already. Defaults to 'main'."
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
echo "[    Info ] Output location:"
echo "            ${output_dir}"

tag_version="main"
if [ ! -z $2 ]; then
    tag_version=$2
fi
if [ -d "${tag_version}" ]; then
    tag_version=$(realpath "${tag_version}")
    echo "[    Info ] API Bridge location (local):"
else
    echo "[    Info ] API Bridge tag:"
fi
echo "            ${tag_version}"
echo

echo "[    Info ] Creating temporary working location..."
echo -n "            "
tmp_dir=$(mktemp -d)
cd $tmp_dir
echo $tmp_dir

# prepare the doc sources

if [ -d "${tag_version}" ]; then
    echo "[    Info ] Retrieving API Bridge project..."
    cp -R "${tag_version}" $tmp_dir
    if [ -d "${tmp_dir}/api-bridge" ]; then
        mv "${tmp_dir}/api-bridge" "${tmp_dir}/api_bridge"
    elif [ ! -d "${tmp_dir}/apibridge" ]; then
        mv "${tmp_dir}/apibridge" "${tmp_dir}/api_bridge"
    elif [ ! -d "${tmp_dir}/api_bridge" ]; then
        echo "[   Error ] Failed to retrieve API Bridge."
        exit 1
    fi
else
    echo "[    Info ] Cloning API Bridge project..."
    pushd . > /dev/null 2>&1
    git clone https://github.com/hebench/api-bridge.git api_bridge
    if [ ! -d "${tmp_dir}/api_bridge" ]; then
        echo "[   Error ] Failed cloning API Bridge project."
        exit 1
    fi
    cd ${tmp_dir}/api_bridge
    git checkout ${tag_version}
    popd > /dev/null 2>&1
fi

echo "[    Info ] Configuring doxygen..."
cp -R "${script_path}/../docsrc" $tmp_dir
mv $tmp_dir/docsrc/DoxygenLayout.xml.in $tmp_dir/DoxygenLayout.xml
mv $tmp_dir/docsrc/doxyfile.in $tmp_dir/doxyfile

#tmp_sed_text="s;@API_BRIDGE_LOCATION;./api_bridge;g"
#sed -i "${tmp_sed_text}" $tmp_dir/doxyfile
tmp_sed_text="s;@OUTPUT_DIR;${output_dir};g"
sed -i "${tmp_sed_text}" $tmp_dir/doxyfile

echo "[    Info ] Copying documentation source files..."

pushd . > /dev/null 2>&1

if [ ! -f "${script_path}/../docsrc/doxyinputs.in" ]; then
    echo "[   ERROR ] File with input sources not found. Expected at:"
    echo "            \"${script_path}/../docsrc/doxyinputs.in\""
    exit 1
fi
# read inputs: 1 input source (dir or file) per line
mapfile -t EXTERNAL_INPUTS < "${script_path}/../docsrc/doxyinputs.in"

# input sources are relative to the `doxyinputs.in` file
cd "${script_path}/../docsrc/"

echo "" >> $tmp_dir/doxyfile
echo -n "INPUT = " >> $tmp_dir/doxyfile

# copy each input source to temp and add the source to the doxyfile
for (( i=0; i<${#EXTERNAL_INPUTS[@]}; i++ ));
do
    EXTERNAL_INPUTS[$i]=$(realpath ${EXTERNAL_INPUTS[$i]})
    if [ -d ${EXTERNAL_INPUTS[$i]} ]; then
        cp -R "${EXTERNAL_INPUTS[$i]}" $tmp_dir
        # add the source directory to the doxyfile
        echo "        ./$(basename ${EXTERNAL_INPUTS[$i]})/ \\" >> $tmp_dir/doxyfile
    elif [ -f ${EXTERNAL_INPUTS[$i]} ]; then
        cp "${EXTERNAL_INPUTS[$i]}" $tmp_dir
        # add the source file to the doxyfile
        echo "        ./$(basename ${EXTERNAL_INPUTS[$i]}) \\" >> $tmp_dir/doxyfile
    else
        echo "Input source not found:"
        echo "\"${EXTERNAL_INPUTS[$i]}\""
        exit 1
    fi
    echo "            \"${EXTERNAL_INPUTS[$i]}\""
done
echo "        ./api_bridge/hebench/" >> $tmp_dir/doxyfile

popd > /dev/null 2>&1

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

