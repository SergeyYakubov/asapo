#!/usr/bin/env bash

cd /asapo/build
cmake -DCMAKE_BUILD_TYPE="Debug" ..
cd worker && make
cd ../producer && make

