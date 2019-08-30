{
  "Mode": "static",
  "Receiver": {
    "StaticEndpoints": [
      "localhost:22001"
    ],
    "MaxConnections": 32
  },
  "Broker": {
    "StaticEndpoint": "localhost:5005"
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug"
}


