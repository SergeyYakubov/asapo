#!/usr/bin/env bash

set -e

yum install -y libibverbs librdmacm librdmacm-devel

wget https://github.com/ofiwg/libfabric/archive/v1.11.0.tar.gz
tar xzf v1.11.0.tar.gz
rm v1.11.0.tar.gz

cd libfabric-1.11.0
./autogen.sh
./configure
make -j$(nproc)
make install
cd -
rm -rf libfabric-1.11.0
