{
  "MonitorDbAddress":"influxdb.service.asapo:8086",
  "MonitorDbName": "asapo_receivers",
  "BrokerDbAddress":"mongo.service.asapo:27017",
  "AuthorizationServer": "asapo-authorizer.service.asapo:5007",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "DataServer": {
    "NThreads": 2,
    "ListenPort": {{ env "NOMAD_PORT_recv_ds" }}
  },
  "DataCache": {
    "Use": true,
    "SizeGB": 30,
    "ReservedShare": 10
  },
  "Tag": "{{ env "NOMAD_ADDR_recv" }}",
  "WriteToDisk":true,
  "WriteToDb":true,
  "LogLevel": "{{ keyOrDefault "receiver_log_level" "info" }}",
  "RootFolder" : "/var/lib/receiver/data"
}
