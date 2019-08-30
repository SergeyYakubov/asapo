{
  "MonitorDbAddress":"localhost:8086",
  "MonitorDbName": "db_test",
  "BrokerDbAddress":"localhost:27017",
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
  "WriteToDb": true,
  "LogLevel" : "debug",
  "RootFolder" : "/tmp/asapo/receiver/files"
}
