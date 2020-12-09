#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DNUMPY_VERSION=0   \
    -DLIBCURL_DIR=/curl \
    ..
make package
cd consumer/api/python/source_dist_linux && make python-rpm-consumer && make python3-rpm-consumer && cd -
cd producer/api/python/source_dist_linux && make python-rpm-producer && make python3-rpm-producer


