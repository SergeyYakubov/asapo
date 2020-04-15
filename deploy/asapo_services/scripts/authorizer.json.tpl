{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"beamtimeId":"asapo_test","beamline":"test","core-path":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test/2019/data/asapo_test"},
  {"beamtimeId":"asapo_test1","beamline":"test1","core-path":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test1/2019/data/asapo_test1"},
  {"beamtimeId":"asapo_test2","beamline":"test2","core-path":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test2/2019/data/asapo_test2"}],
  "RootBeamtimesFolder":"{{ env "NOMAD_META_offline_dir" }}",
  "CurrentBeamlinesFolder":"{{ env "NOMAD_META_online_dir" }}",
  "SecretFile":"/local/secret.key",
  "TokenDurationMin":600
}
