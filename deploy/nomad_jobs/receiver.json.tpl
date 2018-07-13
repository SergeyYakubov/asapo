{
  "MonitorDbAddress":"influxdb.service.asapo:8086",
  "MonitorDbName": "asapo_receivers",
  "BrokerDbAddress":"{{ env "attr.unique.network.ip-address" }}:27015",
  "AuthorizationServer": "asapo-authorizer.service.asapo:5007",
  "AuthorizationInterval": 10000,
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "Tag": "{{ env "NOMAD_ADDR_recv" }}",
  "WriteToDisk":true,
  "WriteToDb":true,
  "LogLevel" : "info",
  "RootFolder" : "/var/lib/receiver/data"
}
