#!/usr/bin/env bash

cd /asapo/build
cmake -DCMAKE_BUILD_TYPE="Release" -DLIBCURL_DIR=/curl -DBUILD_PYTHON_DOCS=ON ..
cd consumer && make
cd ../producer && make
cd ../sphinx && make

