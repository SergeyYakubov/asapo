#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CORE=OFF \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DLIBCURL_DIR=/curl \
    ..
make package
cd consumer/api/python/source_dist_linux && make python-rpm && make python3-rpm && cd -
cd producer/api/python/source_dist_linux && make python-rpm && make python3-rpm


