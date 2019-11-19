{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32,
    "UseIBAddress": false
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug",
}


