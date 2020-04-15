{
  "Mode": "static",
  "Receiver": {
    "StaticEndpoints": [
      "127.0.0.1:22001"
    ],
    "MaxConnections": 32
  },
  "Broker": {
    "StaticEndpoint": "localhost:5005"
  },
  "Mongo": {
    "StaticEndpoint": "127.0.0.1:27017"
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug"
}


