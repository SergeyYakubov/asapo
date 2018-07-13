{
  "BrokerDbAddress":"{{ env "attr.unique.network.ip-address" }}:27016",
  "MonitorDbAddress":"influxdb.service.asapo:8086",
  "MonitorDbName": "asapo_brokers",
  "port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/secrets/secret.key"
}