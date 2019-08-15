from ubuntu:16.04

ENV GOPATH /tmp

RUN apt update && apt install -y cmake g++ golang zlib1g-dev python cython python-numpy python3 cython3 python3-numpy git wget


ADD install_curl.sh install_curl.sh
RUN ./install_curl.sh /curl


ADD build.sh /bin/build.sh
