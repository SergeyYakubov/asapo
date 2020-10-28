#!/usr/bin/env bash

# rdma-core

wget https://github.com/linux-rdma/rdma-core/releases/download/v31.0/rdma-core-31.0.tar.gz

tar -xvf rdma-core-31.0.tar.gz
cd rdma-core-31.0
bash build.sh
cd ..
rm -f rdma-core-31.0.tar.gz
# rdma-core cannot be installed and some files in build are just symlinks
# therefore the folder is used as is and cannot be removed

# libfabric

wget https://github.com/ofiwg/libfabric/releases/download/v1.11.0/libfabric-1.11.0.tar.bz2

tar -xvf libfabric-1.11.0.tar.bz2
cd libfabric-1.11.0
./configure CPPFLAGS=-I/rdma-core-31.0/build/include LDFLAGS=-L/rdma-core-31.0/build/lib
make LD_LIBRARY_PATH=/rdma-core-31.0/build/lib  # not sure why the flag is necessary
make install
cd ..
rm -f libfabric-1.11.0.tar.bz2
rm -rf libfabric-1.11.0
