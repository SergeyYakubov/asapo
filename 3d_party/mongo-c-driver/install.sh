#!/usr/bin/env bash

cd $1
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.9.0/mongo-c-driver-1.9.0.tar.gz
tar xzf mongo-c-driver-1.9.0.tar.gz
cd mongo-c-driver-1.9.0
./configure --disable-automatic-init-and-cleanup --enable-static=yes --enable-shared=no
make
#sudo make install




