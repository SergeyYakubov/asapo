{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32,
    "UseIBAddress": false
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug",
  "Monitoring": {
    "StaticEndpoint": "127.0.0.1:5020"
  },
  "Mongo": {
    "StaticEndpoint": "127.0.0.1:27017"
  }
}


