#!/usr/bin/env bash


set -e

if [[ p$1 == "p" ]]; then
 echo "install folder missing"
 exit 1
fi


cd $1
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.2/mongo-c-driver-1.17.2.tar.gz
tar xzf mongo-c-driver-1.17.2.tar.gz
cd mongo-c-driver-1.17.2

#you can add -DENABLE_ZSTD=OFF to disable lzstd
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SSL=OFF -DENABLE_SASL=OFF  -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DMONGOC_ENABLE_STATIC=ON .
make -j 4
#sudo make install




