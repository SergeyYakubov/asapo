{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "AuthorizationServer": "localhost:8400/asapo-authorizer",
  "PerformanceDbServer": "localhost:8086",
  "MonitorPerformance": true,
  "CheckResendInterval":0,
  "PerformanceDbName": "db_test",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"debug"
}