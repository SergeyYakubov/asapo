{
  "BrokerDbAddress":"127.0.0.1:27017",
  "MonitorDbAddress": "localhost:8086",
  "MonitorDbName": "db_test",
  "port":{{ env "NOMAD_PORT_broker" }},
  "LogLevel":"info",
  "SecretFile":"broker_secret.key"
}