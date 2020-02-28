{
  "AdvertiseIP": "{{ if or (env "meta.ib_address") "none" | regexMatch "none" }}{{ env "NOMAD_IP_recv" }}{{ else }}{{ env "meta.ib_address" }}{{ end }}",
  "PerformanceDbServer":"localhost:8400/influxdb",
  "PerformanceDbName": "asapo_receivers",
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/discovery",
  "AuthorizationServer": "localhost:8400/authorizer",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "DataServer": {
    "NThreads": {{ env "NOMAD_META_receiver_dataserver_nthreads" }},
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }}
  },
  "DataCache": {
    "Use": true,
    "SizeGB": {{ env "NOMAD_META_receiver_dataserver_cache_size" }},
    "ReservedShare": 10
  },
  "Tag": "{{ env "attr.unique.hostname" }}",
  "WriteToDisk":true,
  "ReceiveToDiskThresholdMB": {{ env "NOMAD_META_receiver_receive_to_disk_threshold" }},
  "WriteToDb":true,
  "LogLevel": "{{ keyOrDefault "receiver_log_level" "info" }}"
}
