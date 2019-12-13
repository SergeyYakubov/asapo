FROM yakser/asapo-nomad-cluster

MAINTAINER DESY IT

COPY scripts/ /var/run/asapo/

RUN cd /var/run/asapo && terraform init
