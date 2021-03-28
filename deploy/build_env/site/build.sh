#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DBUILD_CLIENTS_ONLY=ON \
    -DBUILD_PYTHON=OFF   \
    -BUILD_ASAPO_SITE=ON   \
    -DLIBCURL_DIR=/curl \
    ..
cd docs/site
npm install
make site


