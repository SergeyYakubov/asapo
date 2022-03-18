FROM debian:bullseye

RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		ca-certificates \
		curl \
		librdkafka++1 \
		librdkafka1 \
		libfabric1 \
		libmongoc-1.0-0 \
	; \
	rm -rf /var/lib/apt/lists/*

COPY receiver /

CMD ["/receiver","/var/lib/receiver/config.json"]
