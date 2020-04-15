from centos:7

ENV GOPATH /tmp

RUN yum update -y && yum install -y cmake g++ golang zlib1g-dev python cython python-numpy python3 cython3 python3-numpy git wget

RUN yum -y install gcc

RUN yum -y install epel-release make

ADD install_curl.sh install_curl.sh
RUN ./install_curl.sh /curl

RUN yum install -y python-pip python3-pip && pip install --upgrade cython && pip3 install --upgrade cython
RUN pip3 install sphinx

ADD build.sh /bin/build.sh

RUN yum -y install gcc-c++

ADD install_cmake.sh install_cmake.sh
RUN ./install_cmake.sh

RUN yum -y install zlib-devel

RUN yum -y install python3-numpy python-numpy

RUN yum -y install numpy

RUN yum -y install python-devel python36-devel
RUN yum -y install glibc-static libstdc++-static