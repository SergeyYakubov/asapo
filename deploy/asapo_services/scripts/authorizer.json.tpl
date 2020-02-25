{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"beamtimeId":"asapo_test","beamline":"test","core-path":"/var/lib/receiver/data/test_facility/gpfs/test/2019/data/asapo_test"},
  {"beamtimeId":"asapo_test1","beamline":"test1","core-path":"/var/lib/receiver/data/test_facility/gpfs/test1/2019/data/asapo_test1"},
  {"beamtimeId":"asapo_test2","beamline":"test2","core-path":"/var/lib/receiver/data/test_facility/gpfs/test2/2019/data/asapo_test2"}],
  "SecretFile":"/local/secret.key"
}
