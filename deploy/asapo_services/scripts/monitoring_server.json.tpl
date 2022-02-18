{
    "ThisClusterName": "asapo",
    "ServerPort": {{ env "NOMAD_PORT_monitoring_server" }},
    "LogLevel": "{{ keyOrDefault "log_level" "debug" }}",
    "InfluxDbUrl":"http://localhost:8400/influxdb",
    "InfluxDbDatabase": "asapo_monitoring"
}
