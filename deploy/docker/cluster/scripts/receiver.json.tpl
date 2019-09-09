{
  "PerformanceDbServer":"localhost:8400/influxdb",
  "PerformanceDbName": "asapo_receivers",
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "AuthorizationServer": "localhost:8400/authorizer",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "DataServer": {
    "NThreads": 2,
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }}
  },
  "DataCache": {
    "Use": true,
    "SizeGB": {{ env "NOMAD_META_receiver_dataserver_cache_size" }},
    "ReservedShare": 10
  },
  "Tag": "{{ env "NOMAD_ADDR_recv" }}",
  "WriteToDisk":true,
  "WriteToDb":true,
  "LogLevel": "{{ keyOrDefault "receiver_log_level" "info" }}",
  "RootFolder" : "/var/lib/receiver/data"
}
