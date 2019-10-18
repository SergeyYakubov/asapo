{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32,
    "UseIBAddress": {{ keyOrDefault "use_ib_for_receiver" "false" }}
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel": "{{ keyOrDefault "log_level" "info" }}"
}


