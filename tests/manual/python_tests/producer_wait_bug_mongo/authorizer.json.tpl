{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"BeamtimeId":"asapo_test","Beamline":"test"},
  {"BeamtimeId":"asapo_test1","Beamline":"test1"},
  {"BeamtimeId":"asapo_test2","Beamline":"test2"}],
  "SecretFile":"auth_secret.key"
}


