{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "PerformanceDbServer": "localhost:8086",
  "CheckResendInterval":0,
  "PerformanceDbName": "db_test",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"auth_secret.key"
}