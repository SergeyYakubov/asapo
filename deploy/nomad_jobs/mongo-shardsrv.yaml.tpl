sharding:
  clusterRole: shardsvr
net:
  bindIp: {{ env "attr.unique.network.ip-address" }}
  port: {{ env "NOMAD_PORT_shard" }}
