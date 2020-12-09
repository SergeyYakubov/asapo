#!/usr/bin/env bash

set -e

cd /asapo/build
cmake \
    -DCMAKE_BUILD_TYPE="Release" \
    -DENABLE_LIBFABRIC=on \
    -DLIBCURL_DIR=/curl \
    -DBUILD_PYTHON_DOCS=ON \
    -DBUILD_EVENT_MONITOR_PRODUCER=ON \
    ..
cd consumer/api/python/source_dist_linux && make python-dist
cd ../producer/api/python/source_dist_linux && make python-dist-producer
cd ../docs/sphinx && make

