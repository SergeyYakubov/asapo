{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "PerformanceDbServer":"localhost:8400/influxdb",
  "PerformanceDbName": "asapo_brokers",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/local/secret.key"
}
