#!/bin/bash

OUT_DIR=$1
ASAPO_MINIMUM_COVERAGE=$2

export GOPATH=$GOPATH:$3

echo $OUT_DIR

mapfile -t PACKAGES < <( find ./src -type d -not -path '*/\.*' )

echo "mode: count" > $OUT_DIR/coverage-all.out
for pkg in ${PACKAGES[@]}
do
#	echo $pkg
	go test -coverprofile=$OUT_DIR/coverage.out -tags test $pkg #>/dev/null 2>&1
	tail -n +2 $OUT_DIR/coverage.out >> $OUT_DIR/coverage-all.out #2>/dev/null
done

coverage=`go tool cover -func=$OUT_DIR/coverage-all.out | grep total | cut -d ")" -f 2 | cut -d "." -f 1`

#firefox ./coverage.html &
go tool cover -html=$OUT_DIR/coverage-all.out -o ${OUT_DIR}/coverage.html
rm -rf ${OUT_DIR}/coverage-all.out ${OUT_DIR}/coverage.out
if (( coverage < ASAPO_MINIMUM_COVERAGE )); then
	exec >&2
	echo
	echo "*****"
	echo
	echo coverage is ${coverage}% - less than required ${ASAPO_MINIMUM_COVERAGE}%
	echo	
	echo "*****"
	echo
    exit 1
fi

