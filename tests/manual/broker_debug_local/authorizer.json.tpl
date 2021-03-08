{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"beamtimeId":"asapo_test","beamline":"test","Year":"2019","Facility":"test_facility"},
  {"beamtimeId":"asapo_test1","beamline":"test1","Year":"2019","Facility":"test_facility"},
  {"beamtimeId":"asapo_test2","beamline":"test2","Year":"2019","Facility":"test_facility"}],
  "UserSecretFile":"auth_secret.key",
  "AdminSecretFile":"auth_secret_admin.key"
}


