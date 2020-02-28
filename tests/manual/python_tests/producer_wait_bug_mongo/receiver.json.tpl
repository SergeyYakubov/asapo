{
  "AdvertiseIP": "127.0.0.1",
  "PerformanceDbServer":"localhost:8086",
  "PerformanceDbName": "db_test",
  "DatabaseServer":"localhost:27017",
  "DiscoveryServer": "localhost:8400/discovery",
  "DataServer": {
    "NThreads": 2,
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }}
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
