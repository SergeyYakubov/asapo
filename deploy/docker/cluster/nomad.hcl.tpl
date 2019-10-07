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
  bootstrap_expect = $n_servers
}

data_dir = "/var/nomad"

client {
  enabled       = true
  alloc_dir="$nomad_alloc_dir"
}


plugin "docker" {
  config {
    endpoint = "$docker_endpoint"

    tls {
      cert = "/etc/nomad/cert.pem"
      key  = "/etc/nomad/key.pem"
      ca   = "/etc/nomad/ca.pem"
    }

    allow_privileged = true

  }
}

