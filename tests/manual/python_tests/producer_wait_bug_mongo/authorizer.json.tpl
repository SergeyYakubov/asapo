{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"BeamtimeId":"asapo_test","Beamline":"test","Year":"2019","Facility":"test_facility"},
  {"BeamtimeId":"asapo_test1","Beamline":"test1","Year":"2019","Facility":"test_facility"},
  {"BeamtimeId":"asapo_test2","Beamline":"test2","Year":"2019","Facility":"test_facility"}],
  "SecretFile":"auth_secret.key"
}


