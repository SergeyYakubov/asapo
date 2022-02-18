{
  "Port": {{ env "NOMAD_PORT_fts" }},
  "LogLevel":"debug",
  "SecretFile":"/local/secret.key",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "MonitorPerformance": true,
  "MonitoringServerUrl": "auto"
}
