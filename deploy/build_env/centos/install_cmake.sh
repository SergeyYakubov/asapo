#!/usr/bin/env bash

set -e

yum install -y openssl-devel

wget https://cmake.org/files/v3.21/cmake-3.21.1.tar.gz

tar zxvf cmake-3.*
rm cmake-3.*.tar.gz

cd cmake-3.*
./bootstrap --prefix=/usr/local
make -j$(nproc)
make install

cd ..
rm -rf cmake-3.*

echo "Version check global cmake --version"
cmake --version

echo "Version check usr local bin cmake --version"
/usr/local/bin/cmake --version
