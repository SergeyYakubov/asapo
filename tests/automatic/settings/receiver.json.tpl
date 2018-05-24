{
  "MonitorDbAddress":"localhost:8086",
  "MonitorDbName": "db_test",
  "BrokerDbAddress":"localhost:27017",
  "BrokerDbName": "test_run",
  "ListenPort": {{ env "NOMAD_PORT_recv" }},
  "WriteToDisk":true,
  "WriteToDb":true,
  "LogLevel" : "debug"
}
