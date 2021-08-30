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
  "Monitoring": {
    "StaticEndpoint": "127.0.0.1:5020"
  },
  "Mongo": {
    "StaticEndpoint": "asapo-services:27017"
  },
  "Port": {{ env "NOMAD_PORT_discovery" }},
  "LogLevel":"debug"
}


