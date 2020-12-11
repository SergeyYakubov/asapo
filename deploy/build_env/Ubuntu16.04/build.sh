#!/usr/bin/env bash

set -e

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=on \
    -DLIBCURL_DIR=/curl \
    -DBUILD_PYTHON_DOCS=ON \
    -DBUILD_PYTHON_PACKAGES=source \
    -DBUILD_CLIENTS_ONLY=ON \
    ..
make
#cd consumer/api/python/dist_linux && make python-dist-consumer
#cd ../producer/api/python/dist_linux && make python-dist-producer
#cd ../docs/sphinx && make

