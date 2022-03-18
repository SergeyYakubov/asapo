FROM debian:bullseye

RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		ca-certificates \
		curl \
		gnupg \
	; \
	rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://apt.releases.hashicorp.com/gpg  | gpg --dearmor > /usr/share/keyrings/nomad-hashicorp-archive-keyring.gpg
RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/nomad-hashicorp-archive-keyring.gpg] https://apt.releases.hashicorp.com bullseye main" >> /etc/apt/sources.list

RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		supervisor \
		gnupg-agent \
		dnsutils \
		nomad \
		consul \
		terraform \
		iproute2 \
	; \
	rm -rf /var/lib/apt/lists/*

ADD supervisord.conf /etc/

RUN mkdir -p /var/log/supervisord/ /etc/consul.d /etc/nomad.d /var/nomad /var/consul

COPY scripts/ /var/asapo/

RUN cd /var/asapo && terraform init

COPY jobs-* asapo-wait-service /usr/bin/

COPY *.py  /etc/asapo/
COPY *.hcl.tpl  /etc/asapo/

RUN chmod -R og+wrX /var/log /var/lib /var/tmp /var/asapo /var/nomad /var/consul /etc/consul.d /etc/nomad.d

ENTRYPOINT ["supervisord", "--configuration", "/etc/supervisord.conf"]
