FROM gitlab.desy.de:5555/asapo/asapo/asapo-nomad-consul:latest

COPY scripts/ /var/asapo/

RUN chmod -R og+wrX /var/asapo

RUN cd /var/asapo && terraform init
