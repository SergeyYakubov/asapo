{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "PerformanceDbServer":"localhost:8400/influxdb",
  "CheckResendInterval":10,
  "PerformanceDbName": "asapo_brokers",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"/local/secret.key"
}
