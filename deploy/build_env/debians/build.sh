#!/usr/bin/env bash

set -e

cd /asapo
rm -rf build || true
mkdir build
cd build

cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=ON \
    -DINSTALL_EXAMPLES=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DPACK_STATIC_CURL_LIB=/curl/lib/libcurl.a \
    -DBUILD_PYTHON=OFF   \
    -DCPACK_PACKAGE_NAME="asapo-dev" \
    -DCPACK_GENERATOR="DEB" \
    ..
cmake .. #second time for to correctly build deb packages
make -j 4
make package


if [ "$OS" = "ubuntu18.04" ]; then
  BUILD_PYTHON_DOCS=ON
else
  BUILD_PYTHON_DOCS=OFF
fi

if [ $OS == "debian9.13" -o $OS == "debian10.7" -o $OS == "ubuntu16.04" -o $OS == "ubuntu18.04" ]; then
  BUILD_PYTHON2_PACKAGES=ON
else
  BUILD_PYTHON2_PACKAGES=OFF
fi

#switch to static curl for Python packages
rm CMakeCache.txt
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DLIBCURL_DIR=/curl  \
    -DENABLE_LIBFABRIC=ON \
    -DBUILD_CLIENTS_ONLY=ON \
    -DNUMPY_VERSION=0   \
    -DBUILD_PYTHON=ON   \
    -DPACKAGE_RELEASE_SUFFIX=$OS \
    -DBUILD_PYTHON2_PACKAGES=$BUILD_PYTHON2_PACKAGES \
    -DBUILD_PYTHON_PACKAGES="source;deb"   \
    -DBUILD_PYTHON_DOCS=$BUILD_PYTHON_DOCS \
    ..
make -j 1
