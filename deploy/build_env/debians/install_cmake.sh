#!/usr/bin/env bash

wget https://cmake.org/files/v3.10/cmake-3.10.0.tar.gz

tar zxvf cmake-3.10.0.tar.gz
cd cmake-3.10.0
./bootstrap --prefix=/usr/local
make -j$(nproc)
make install

cd ..
rm -rf cmake-3*

cmake --version

/usr/local/bin/cmake --version
