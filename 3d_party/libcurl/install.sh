#!/usr/bin/env bash

cd $1
wget https://curl.haxx.se/download/curl-7.58.0.tar.gz
tar xzf curl-7.58.0.tar.gz
cd curl-7.58.0
./configure --without-ssl --disable-shared --disable-manual --disable-ares --disable-cookies \
--disable-crypto-auth --disable-ipv6 --disable-proxy --disable-unix-sockets \
--without-libidn --without-librtmp --without-zlib --disable-ldap \
--disable-libcurl-option --prefix=`pwd`/../
make
make install
cd -
rm -rf bin share curl-7.58.0
rm curl-7.58.0.tar.gz
