#!/bin/bash

# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

# Reads a version file in the HEBench version format and outputs the
# full formatted version to stdout.

#exit on any command failure
set -e
tmp_dir=""

error_exit()
{
if [ -d $tmp_dir ]; then
    rm -rf $tmp_dir
fi
if [ "$1" != "0" ]; then
	echo "Unexpected error occurred during last operation."
fi
}

# call exit function on exit to display success/error message
trap 'error_exit $?' EXIT

if [ -z $1 ] || [ "${1}" == "-h" ] || [ "${1}" == "--help" ]; then
    echo "HEBench version parser."
    echo "  Usage:"
    echo "    script <version_filename>"
    echo
    echo "  Positional arguments:"
    echo "    - version_filename: file from which to parse the version tag."
    echo
    if [ -z $1 ]; then
        exit 1
    else
        exit 0
    fi
fi

VERSION_FILE=$1

if [ ! -f "${VERSION_FILE}" ]; then
    echo "File not found: ${VERSION_FILE}"
    exit 1
fi

mapfile -t VERSION_LINES < ${VERSION_FILE}
if [ ${#VERSION_LINES[@]} -lt 1 ]; then
    echo "Invalid version file format."
    exit 1
fi

if [ ${#VERSION_LINES[@]} -gt 1 ] && [ "${VERSION_LINES[0]}" == "v" ]; then
    #v2
    TAG="v${VERSION_LINES[1]}"
    max_lines=${#VERSION_LINES[@]}
    if [ $max_lines -gt 4 ]; then
        max_lines=4
    fi
    #v2.5.3
    for i in $(seq 2 $((max_lines - 1))); do
        TAG="${TAG}.${VERSION_LINES[${i}]}"
    done
    #v2.5.3-rc
    if [ ${#VERSION_LINES[@]} -gt 4 ]; then
        TAG="${TAG}-${VERSION_LINES[4]}"
    fi
    echo $TAG
else
    echo "${VERSION_LINES[0]}"
fi

exit 0

