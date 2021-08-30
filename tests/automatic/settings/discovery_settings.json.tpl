{
  "Mode": "consul",
  "Receiver": {
    "MaxConnections": 32,
    "UseIBAddress": false
  },
  "Monitoring": {
    "StaticEndpoint": "127.0.0.1:5020"
  },
  "Mongo": {
    "StaticEndpoint": "127.0.0.1:27017"
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug"
}


