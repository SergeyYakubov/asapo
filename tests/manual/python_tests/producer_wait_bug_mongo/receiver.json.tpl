{
  "PerformanceDbServer":"localhost:8086",
  "PerformanceDbName": "db_test",
  "DatabaseServer":"localhost:27017",
  "DiscoveryServer": "localhost:8400/discovery",
  "DataServer": {
    "AdvertiseURI": "127.0.0.1",
    "NThreads": 2,
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }},
    "NetworkMode": ["tcp"]
  },
  "DataCache": {
    "Use": true,
    "SizeGB": 1,
    "ReservedShare": 10
  },
  "AuthorizationServer": "localhost:8400/authorizer",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "Tag": "{{ env "NOMAD_ADDR_recv" }}",
  "WriteToDisk": true,
  "ReceiveToDiskThresholdMB":50,
  "WriteToDb": true,
  "LogLevel" : "debug"
}
