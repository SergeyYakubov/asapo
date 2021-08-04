{
  "DatabaseServer":"auto",
  "MonitoringServerUrl":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "PerformanceDbServer": "localhost:8086",
  "MonitorPerformance": true,
  "PerformanceDbName": "db_test",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "CheckResendInterval":10,
  "UserSecretFile":"auth_secret.key",
  "AdminSecretFile":"auth_secret_admin.key"
}
