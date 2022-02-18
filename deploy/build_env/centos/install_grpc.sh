#!/usr/bin/env bash

set -e

git clone --recurse-submodules -b v1.39.0 https://github.com/grpc/grpc

cd grpc

# Install gRPC
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      ../..
make -j$(nproc)
make install

cd ../..
# Install abseil
mkdir -p third_party/abseil-cpp/cmake/build
cd third_party/abseil-cpp/cmake/build
cmake -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
      ../..
make -j$(nproc)
make install
