{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "PerformanceDbServer":"localhost:8400/influxdb",
  "PerformanceDbName": "asapo_brokers",
  "port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/secrets/secret.key"
}