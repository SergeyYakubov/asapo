{
  "BrokerDbAddress":"mongo.service.asapo:27017",
  "MonitorDbAddress":"influxdb.service.asapo:8086",
  "MonitorDbName": "asapo_brokers",
  "port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/secrets/secret.key"
}