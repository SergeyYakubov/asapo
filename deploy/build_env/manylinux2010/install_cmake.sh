#!/usr/bin/env bash

wget https://cmake.org/files/v3.21/cmake-3.21.1.tar.gz

tar zxvf cmake-3.*
cd cmake-3.*
./bootstrap --prefix=/usr/local
make -j$(nproc)
make install

cmake --version

/usr/local/bin/cmake --version
