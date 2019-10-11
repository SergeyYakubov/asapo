"bind_addr" = "$advertise_ip"

enable_script_checks = true

recursors = $recursors

domain = "asapo"

datacenter = "dc1"
data_dir = "/var/consul"
log_level = "INFO"

enable_syslog =  false
enable_debug =  false
ui = true

addresses =  {
"http" =  "0.0.0.0"
}

node_meta = {
  ib_address = "$ib_address"
}

server = $is_server
$bootstrap_expect_string

rejoin_after_leave = true
retry_join = $server_adresses


