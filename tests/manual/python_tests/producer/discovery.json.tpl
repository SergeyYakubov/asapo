{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32,
    "UseIBAddress": false
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug",
  "Mongo": {
    "StaticEndpoint": "127.0.0.1:27017"
  }
}


