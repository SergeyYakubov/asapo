{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel": "{{ keyOrDefault "log_level" "info" }}"
}


