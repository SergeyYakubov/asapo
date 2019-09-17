"data_dir" = "$data_dir"

advertise {
  http = "$advertise_ip"
  rpc = "$advertise_ip"
  serf = "$advertise_ip"
}

acl {
  enabled = true
}

server {
  enabled          = $is_server
  bootstrap_expect = 3
}

client {
#  network_interface = "$network_interface"
  enabled       = true
  node_class = "$node_class"
}
