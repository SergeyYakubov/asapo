{
  "MonitorDbAddress":"influxdb.service.asapo:8086",
  "MonitorDbName": "asapo_receivers",
  "BrokerDbAddress":"localhost:27017",
  "AuthorizationServer": "authorizer.service.asapo:8400",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "Tag": "{{ env "NOMAD_ADDR_recv" }}",
  "WriteToDisk":true,
  "WriteToDb":true,
  "LogLevel" : "info",
  "RootFolder" : "/var/lib/receiver/data"
}
