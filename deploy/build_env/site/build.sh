#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DBUILD_CLIENTS_ONLY=ON \
    -DBUILD_PYTHON=OFF   \
    -DBUILD_ASAPO_SITE=ON   \
    -DLIBCURL_DIR=/curl \
    -DPython_EXECUTABLE="noop" \
    ..
export HOME="/asapo/build"
cd docs/site
npm install
make site


