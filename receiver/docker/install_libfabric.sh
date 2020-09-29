#!/usr/bin/env bash

apt install -y wget autoconf libtool make rdma-core
wget https://github.com/ofiwg/libfabric/archive/v1.11.0.tar.gz
tar xzf v1.11.0.tar.gz
cd libfabric-1.11.0
./autogen.sh
./configure
make
make install
ldconfig
cd -
rm -rf libfabric-1.11.0
rm v1.11.0.tar.gz
