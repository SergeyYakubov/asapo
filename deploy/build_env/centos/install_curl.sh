#!/usr/bin/env bash

set -e

if [ "$1" = "" ]
then
  echo "Usage: $0 <install path>"
  exit
fi

mkdir -p $1
cd $1
wget --no-check-certificate https://curl.haxx.se/download/curl-7.58.0.tar.gz
tar xzf curl-7.58.0.tar.gz
rm curl-*.tar.gz
cd curl-7.58.0

./configure --without-ssl --disable-shared --disable-manual --disable-ares \
--disable-crypto-auth --disable-ipv6 --disable-proxy --disable-unix-sockets \
--without-libidn --without-librtmp --without-zlib --disable-ldap \
--disable-libcurl-option --prefix=`pwd`/../
make -j$(nproc)
make install

cd -
rm -rf bin share curl-7.58.0
