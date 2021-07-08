#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=1.$OS \
    -DBUILD_PYTHON=OFF   \
    -DCPACK_PACKAGE_NAME="asapo-devel" \
    -DCPACK_GENERATOR="RPM" \
    ..
make -j 4
make package

#switch to static curl for Python packages
rm CMakeCache.txt
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLIBCURL_DIR=/curl  \
    -DENABLE_LIBFABRIC=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DNUMPY_VERSION=0   \
    -DBUILD_PYTHON=ON   \
    -DBUILD_PYTHON_PACKAGES="source;rpm"   \
    -DBUILD_PYTHON_DOCS=$BUILD_PYTHON_DOCS \
    ..
make -j 1

