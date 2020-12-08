#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLIBCURL_DIR=/curl \
    ..
cd consumer && make && make python-rpm && make python3-rpm
cd ../producer && make && make python-rpm && make python3-rpm


