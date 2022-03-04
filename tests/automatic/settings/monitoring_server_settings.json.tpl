{
    "ThisClusterName": "asapo",
    "ServerPort": {{ env "NOMAD_PORT_monitoring_server" }},
    "LogLevel": "debug",
    "InfluxDbUrl": "http://localhost:8086",
    "InfluxDbDatabase": "asapo_monitoring"
}
