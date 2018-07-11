sharding:
  clusterRole: configsvr
replication:
  replSetName: config
net:
  bindIp: {{ env "attr.unique.network.ip-address" }}
  port: {{ env "NOMAD_PORT_config" }}
