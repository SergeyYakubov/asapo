FROM quay.io/pypa/manylinux2010_x86_64

ENV GOPATH /tmp

RUN yum update -y && yum install -y golang wget zlib-devel

ADD install_curl.sh install_curl.sh
RUN ./install_curl.sh /curl

ADD build.sh /bin/build.sh

RUN chmod og+rwX -R /opt

ADD install_cmake.sh install_cmake.sh
RUN ./install_cmake.sh

