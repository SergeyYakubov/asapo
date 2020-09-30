from ubuntu:16.04

ENV GOPATH /tmp

ADD install_curl.sh install_curl.sh
ADD install_cmake.sh install_cmake.sh
ADD install_libfabric.sh install_libfabric.sh

RUN apt update && apt install -y g++ golang zlib1g-dev python cython python-numpy python3 cython3 python3-numpy  \
git wget python-pip python3-pip && \
pip  --no-cache-dir install --upgrade cython && pip3  --no-cache-dir install --upgrade cython && pip3  --no-cache-dir install sphinx && \
./install_curl.sh /curl &&  ./install_cmake.sh && ./install_libfabric.sh \
apt-get purge -y --auto-remove

ADD build.sh /bin/build.sh

