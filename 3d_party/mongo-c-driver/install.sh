#!/usr/bin/env bash

cd $1
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.15.2/mongo-c-driver-1.15.2.tar.gz
tar xzf mongo-c-driver-1.15.2.tar.gz
cd mongo-c-driver-1.15.2

cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SSL=OFF -DENABLE_SASL=OFF -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DMONGOC_ENABLE_STATIC=ON .
make
#sudo make install




