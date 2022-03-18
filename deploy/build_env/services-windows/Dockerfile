FROM golang:1.17.6-bullseye

RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		cmake \
		libfabric-dev \
		librdkafka-dev \
		libmongoc-dev \
		googletest \
		libcurl4-openssl-dev \
		nomad \
	; \
	rm -rf /var/lib/apt/lists/*

