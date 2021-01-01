#!/usr/bin/env bash

wget https://github.com/ofiwg/libfabric/archive/v1.11.0.tar.gz
tar xzf v1.11.0.tar.gz
cd libfabric-1.11.0
./autogen.sh
./configure
make
make install
cd -
rm -rf libfabric-1.11.0
rm v1.11.0.tar.gz
