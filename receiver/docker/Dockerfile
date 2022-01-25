FROM ubuntu:18.04
ADD receiver /
ADD install_libfabric.sh install_libfabric.sh
ADD install_rdkafka.sh install_rdkakfa.sh
RUN apt update && ./install_libfabric.sh && ./install_rdkakfa.sh

CMD ["/receiver","/var/lib/receiver/config.json"]
