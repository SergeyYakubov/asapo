FROM yakser/asapo-nomad-cluster

MAINTAINER DESY IT

COPY scripts/ /var/run/asapo/

COPY asapo-wait-service  /usr/bin/

RUN cd /var/run/asapo asapo && terraform init
