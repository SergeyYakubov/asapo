#!/bin/bash

set -e

TARGET=$1
BINARY_DIR=$2
ASAPO_MINIMUM_COVERAGE=$3

make ${TARGET}

coverage=`cat ${BINARY_DIR}/${TARGET}.txt`
if [ -z "$coverage" ]; then
 coverage=0
fi
if (( coverage < ASAPO_MINIMUM_COVERAGE )); then
	exec >&2
	echo
	echo "*****"
	echo
	echo ${TARGET} coverage is ${coverage}% - less than required ${ASAPO_MINIMUM_COVERAGE}%
	echo	
	echo "*****"
	echo
    exit 1
fi

