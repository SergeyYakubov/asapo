{
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "AuthorizationServer": "localhost:8400/asapo-authorizer",
  "PerformanceDbServer":"localhost:8400/influxdb",
  "MonitorPerformance": {{ env "NOMAD_META_perf_monitor" }},
  "MonitoringServerUrl":"auto",
  "CheckResendInterval":10,
  "PerformanceDbName": "asapo_brokers",
  "Port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info"
}
