FROM quay.io/pypa/manylinux2010_x86_64:2021-04-05-a6ea1ab

ENV GOPATH /tmp

RUN yum update -y && yum install -y golang wget zlib-devel libnl3-devel

ADD install_curl.sh install_curl.sh
RUN ./install_curl.sh /curl

ADD install_cmake.sh install_cmake.sh
RUN ./install_cmake.sh

ADD install_libfabric.sh install_libfabric.sh
RUN ./install_libfabric.sh

RUN chmod og+rwX -R /opt

ADD build.sh /bin/build.sh