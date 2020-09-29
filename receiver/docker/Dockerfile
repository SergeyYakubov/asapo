FROM ubuntu:18.04
ADD receiver /
ADD install_libfabric.sh install_libfabric.sh
RUN apt update && apt install -y librdmacm && ./install_libfabric.sh

CMD ["/receiver","/var/lib/receiver/config.json"]
