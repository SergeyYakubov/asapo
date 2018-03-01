#!/usr/bin/env bash

mkdir -p test
cd test
for i in `seq 0 49`;
do
    echo $i > $i
done
