{
  "BrokerDbAddress":"localhost:8400/mongo",
  "MonitorDbAddress":"localhost:8400/influxdb",
  "MonitorDbName": "asapo_brokers",
  "port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/secrets/secret.key"
}
