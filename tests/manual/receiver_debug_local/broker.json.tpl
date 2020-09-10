{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "PerformanceDbServer": "localhost:8086",
  "PerformanceDbName": "db_test",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "CheckResendInterval":10,
  "SecretFile":"auth_secret.key"
}