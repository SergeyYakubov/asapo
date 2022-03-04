#!/usr/bin/env bash

apt install -y wget build-essential autoconf libtool make zlib1g-dev libzstd-dev

wget https://github.com/edenhill/librdkafka/archive/refs/tags/v1.8.2.tar.gz
tar xzf v1.8.2.tar.gz
cd librdkafka-1.8.2

./configure --enable-zlib --enable-zstd --disable-lz4 --disable-lz4-ext --disable-ssl --disable-gssapi --disable-sasl
make -j 4
make install
ldconfig
cd -
rm -rf librdkafka-1.8.2
rm v1.8.2.tar.gz