{
  "Port": {{ env "NOMAD_PORT_authorizer" }},
  "LogLevel":"debug",
  "AlwaysAllowedBeamtimes":[{"beamtimeId":"asapo_test","beamline":"test","corePath":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test/2019/data/asapo_test", "beamline-path":"{{ env "NOMAD_META_online_dir" }}/test/current"},
  {"beamtimeId":"asapo_test1","beamline":"test1","corePath":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test1/2019/data/asapo_test1", "beamline-path":"{{ env "NOMAD_META_online_dir" }}/test1/current"},
  {"beamtimeId":"asapo_test2","beamline":"test2","corePath":"{{ env "NOMAD_META_offline_dir" }}/test_facility/gpfs/test2/2019/data/asapo_test2", "beamline-path":"{{ env "NOMAD_META_online_dir" }}/test2/current"}],
  "RootBeamtimesFolder":"{{ env "NOMAD_META_offline_dir" }}",
  "CurrentBeamlinesFolder":"{{ env "NOMAD_META_online_dir" }}",
  "UserSecretFile":"/local/secret.key",
  "AdminSecretFile":"/local/secret_admin.key",
  "TokenDurationMin":600,
  "Ldap":
    {
        "Uri" : "{{ env "NOMAD_META_ldap_uri" }}",
        "BaseDn" : "ou=rgy,o=desy,c=de",
        "FilterTemplate" : "(cn=a3__BEAMLINE__-hosts)"
    },
  "DatabaseServer":"auto",
  "DiscoveryServer": "localhost:8400/asapo-discovery"
}
