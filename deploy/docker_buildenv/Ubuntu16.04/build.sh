#!/usr/bin/env bash

cd /asapo/build
cmake -DCMAKE_BUILD_TYPE="Release" -DLIBCURL_DIR=/curl ..
cd worker && make
cd ../producer && make

