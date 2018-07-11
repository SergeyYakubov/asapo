sharding:
  configDB: "config/mongo-config-srv.service.asapo:27010"
net:
  bindIp: {{ env "attr.unique.network.ip-address" }}
  port: {{ env "NOMAD_PORT_mongos" }}
