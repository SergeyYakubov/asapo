#!/usr/bin/env bash

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=1.$OS \
    -DBUILD_PYTHON=OFF   \
    -DLIBCURL_DIR=/curl \
    -DCPACK_PACKAGE_NAME="asapo-devel" \
    -DCPACK_GENERATOR="RPM" \
    ..
make -j 4
make package

cmake -DNUMPY_VERSION=0 -DBUILD_PYTHON=ON -DBUILD_PYTHON_PACKAGES="source;rpm" ..
make

