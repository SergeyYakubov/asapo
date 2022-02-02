{
  "Port": {{ env "NOMAD_PORT_file_transfer" }},
  "LogLevel":"debug",
  "SecretFile":"auth_secret.key",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "MonitorPerformance": true,
  "MonitoringServerUrl": "auto"
}
