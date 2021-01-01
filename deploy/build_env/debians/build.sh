#!/usr/bin/env bash

set -e

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DBUILD_PYTHON=OFF   \
    -DLIBCURL_DIR=/curl \
    -DCPACK_PACKAGE_NAME="asapo-dev" \
    -DCPACK_GENERATOR="DEB" \
    ..
cmake .. #second time for to correctly build deb packages
make
make package


if [ "$OS" = "ubuntu16.04" ]; then
  BUILD_PYTHON_DOCS=ON
else
  BUILD_PYTHON_DOCS=OFF
fi


cmake -DNUMPY_VERSION=0 -DBUILD_PYTHON=ON -DBUILD_PYTHON_PACKAGES="source;deb" -DBUILD_PYTHON_DOCS=$BUILD_PYTHON_DOCS ..
make -j1
