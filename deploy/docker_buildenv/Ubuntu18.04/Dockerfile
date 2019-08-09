from ubuntu:18.04

ENV GOPATH /tmp

RUN apt update && apt install -y cmake g++ libcurl4-openssl-dev golang zlib1g-dev python cython python-numpy python3 cython3 python3-numpy


RUN apt install -y git


ADD build.sh /bin/build.sh