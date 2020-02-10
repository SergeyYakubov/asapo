from ubuntu:16.04

ENV GOPATH /tmp

RUN apt update && apt install -y cmake g++ golang zlib1g-dev python cython python-numpy python3 cython3 python3-numpy git wget


ADD install_curl.sh install_curl.sh
RUN ./install_curl.sh /curl

RUN apt install -y python-pip python3-pip && pip install --upgrade cython && pip3 install --upgrade cython
RUN pip3 install sphinx

ADD install_cmake.sh install_cmake.sh
RUN ./install_cmake.sh


ADD build.sh /bin/build.sh

