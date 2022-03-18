FROM golang:1.17.6-bullseye

RUN curl -fsSL https://apt.releases.hashicorp.com/gpg  | gpg --dearmor > /usr/share/keyrings/nomad-hashicorp-archive-keyring.gpg
RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/nomad-hashicorp-archive-keyring.gpg] https://apt.releases.hashicorp.com bullseye main" >> /etc/apt/sources.list

RUN curl -fsSL https://www.mongodb.org/static/pgp/server-5.0.asc | gpg --dearmor > /usr/share/keyrings/mongodb-archive-keyring.gpg
RUN echo "deb [signed-by=/usr/share/keyrings/mongodb-archive-keyring.gpg] http://repo.mongodb.org/apt/debian buster/mongodb-org/5.0 main" >> /etc/apt/sources.list

RUN echo "deb http://deb.debian.org/debian bullseye-backports main" >> /etc/apt/sources.list

RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		cmake \
		libicu-dev \
		libfabric-dev \
		librdkafka-dev \
		libmongoc-dev \
		libgtest-dev \
		libgmock-dev \
		libcurl4-openssl-dev \
		grpc-proto \
		python3-dev \
		python3-numpy \
		cython3 \
		lcov \
		nomad \
		consul \
		iproute2 \
		nginx-light \
		mongodb-org-server \
		graphviz \
		doxygen \
	; \
	apt-get install -y --no-install-recommends cmake/bullseye-backports; \
	DEBIAN_FRONTEND=noninteractive apt-get install -y slapd; \
	rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://raw.githubusercontent.com/mongodb/mongo/master/debian/init.d > /etc/init.d/mongod
RUN chmod a+x /etc/init.d/mongod

RUN echo "data_dir = \"/opt/nomad/data\"\n\
bind_addr = \"0.0.0.0\"\n\
server {\n\
  enabled = true\n\
  bootstrap_expect = 1\n\
}\n\
client {\n\
  enabled = true\n\
  servers = [\"127.0.0.1\"]\n\
}\n\
plugin \"raw_exec\" {\n\
  config {\n\
    enabled = true\n\
  }\n\
}" > /etc/nomad.d/nomad.hcl

RUN echo "enable_script_checks = true\n\
recursors = [\"8.8.8.8\"]\n\
domain = \"asapo\"\n\
data_dir = \"/opt/consul\"\n\
enable_syslog =  false\n\
enable_debug =  false\n\
ui = true\n\
addresses =  {\n\
"http" =  \"0.0.0.0\"\n\
}\n\
advertise_addr = \"127.0.0.1\"\n\
node_meta = {\n\
  ib_address = \"10.10.0.1\"\n\
}\n\
server=true\n\
bootstrap_expect=1\n\
telemetry = {\n\
    disable_compat_1.9 = false\n\
    prometheus_retention_time = \"24h\"\n\
}" > /etc/consul.d/consul.hcl

RUN echo "include /etc/ldap/schema/core.schema\n\
include /etc/ldap/schema/cosine.schema\n\
include /etc/ldap/schema/nis.schema\n\
modulepath /usr/lib/ldap\n\
moduleload back_bdb.so\n\
pidfile /var/run/slapd/slapd.pid\n\
access to * by * write\n\
access to * by * manage\n\
access to * by * read\n\
allow bind_anon_cred\n\
allow bind_anon_dn\n\
allow update_anon\n\
database    bdb\n\
suffix      \"ou=rgy,o=desy,c=de\"\n\
" > /etc/ldap/slapd.conf

RUN rm -rf /etc/ldap/slapd.d

RUN echo "dn: ou=rgy,o=desy,c=de\n\
objectclass: organizationalUnit\n\
ou: rgy\n\
\n\
dn: ou=netgroup,ou=rgy,o=desy,c=de\n\
objectclass: organizationalUnit\n\
ou: netgroup\n\
\n\
dn: cn=a3p00-hosts,ou=netgroup,ou=rgy,o=desy,c=de\n\
objectClass :top\n\
objectClass :nisNetgroup\n\
cn:a3p00-hosts\n\
description: Netgroup for nodes on PETRA III Beamline P00\n\
nisNetgroupTriple: (blabla,-,)\n\
nisNetgroupTriple: (localhost,-,)\n\
nisNetgroupTriple: (blabla2,-,)\n\
\n\
dn: cn=a3p07-hosts,ou=netgroup,ou=rgy,o=desy,c=de\n\
objectClass :top\n\
objectClass :nisNetgroup\n\
cn:a3p07-hosts\n\
description: Netgroup for nodes on PETRA III Beamline P07\n\
nisNetgroupTriple: (blabla,-,)\n\
nisNetgroupTriple: (localhost,-,)\n\
nisNetgroupTriple: (blabla2,-,)\n\
" > /etc/ldap/record.ldif

RUN echo "\n\
/usr/bin/nomad agent -config /etc/nomad.d &> /dev/null &\n\
/usr/bin/consul agent -config-dir=/etc/consul.d/ &> /dev/null &\n\
service slapd start\n\
service mongod start\n\
\n\
ldapadd -x -D \"ou=rgy,o=desy,c=de\" -f /etc/ldap/record.ldif -h localhost\n\
" > /services_start.sh

CMD ["bash", "/services_start.sh"]
