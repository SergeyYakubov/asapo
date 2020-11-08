{
  "PerformanceDbServer":"localhost:8400/influxdb",
  "PerformanceDbName": "asapo_receivers",
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery",
  "AuthorizationServer": "localhost:8400/asapo-authorizer",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "DataServer": {
    "AdvertiseURI": "{{ if or (env "meta.ib_address") "none" | regexMatch "none" }}{{ env "NOMAD_IP_recv" }}{{ else }}{{ env "meta.ib_address" }}{{ end }}:{{ env "NOMAD_PORT_recv_ds" }}",
    "NThreads": {{ env "NOMAD_META_receiver_dataserver_nthreads" }},
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }},
    "NetworkMode": ["{{ if or (env "meta.ib_address") "none" | regexMatch "none" }}{{ printf "%s" "tcp" }}{{ else }}{{ env "NOMAD_META_receiver_network_modes" |  split "," | join "\",\"" }}{{ end }}"]
  },
  "DataCache": {
    "Use": true,
    "SizeGB": {{ env "NOMAD_META_receiver_dataserver_cache_size" }},
    "ReservedShare": 10
  },
  "Tag": "{{ env "attr.unique.hostname" }}",
  "ReceiveToDiskThresholdMB": {{ env "NOMAD_META_receiver_receive_to_disk_threshold" }},
  "LogLevel": "{{ keyOrDefault "receiver_log_level" "info" }}"
}
