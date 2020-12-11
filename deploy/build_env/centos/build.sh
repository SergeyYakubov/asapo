#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DBUILD_PYTHON=OFF   \
    -DLIBCURL_DIR=/curl \
    ..
make
make package

cmake -DNUMPY_VERSION=0 -DBUILD_PYTHON=ON -DBUILD_PYTHON_PACKAGES="source;rpm" ..
make

