FROM ubuntu:18.04

MAINTAINER DESY IT

ENV CONSUL_VERSION=1.6.0
ENV NOMAD_VERSION=0.9.5

ENV HASHICORP_RELEASES=https://releases.hashicorp.com

RUN apt-get update && apt-get install -y supervisor apt-transport-https \
        ca-certificates \
        curl \
        gnupg-agent \
        software-properties-common

RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -

RUN add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"

RUN apt-get update && apt-get install -y docker-ce-cli wget unzip


RUN set -eux && \
    mkdir -p /tmp/build && \
    cd /tmp/build && \
    wget ${HASHICORP_RELEASES}/consul/${CONSUL_VERSION}/consul_${CONSUL_VERSION}_linux_amd64.zip && \
    unzip -d /bin consul_${CONSUL_VERSION}_linux_amd64.zip && \
    wget ${HASHICORP_RELEASES}/nomad/${NOMAD_VERSION}/nomad_${NOMAD_VERSION}_linux_amd64.zip && \
    unzip -d /bin nomad_${NOMAD_VERSION}_linux_amd64.zip && \
    cd /tmp && \
    rm -rf /tmp/build && \
# tiny smoke test to ensure the binary we downloaded runs
    consul version && \
    nomad version

ADD supervisord.conf /etc/

RUN mkdir -p /var/log/supervisord/

ENTRYPOINT ["supervisord", "--configuration", "/etc/supervisord.conf"]
