advertise {
  http = "$advertise_ip"
  rpc = "$advertise_ip"
  serf = "$advertise_ip"
}

acl {
  enabled = $acl_enabled
}

server {
  enabled = $is_server
  $bootstrap_expect_string
}

data_dir = "/var/nomad"

client {
  enabled       = true
  alloc_dir="$nomad_alloc_dir"
  meta {
      "asapo_service" = $is_asapo_lightweight_service_node
      "ib_address" = "$ib_address"
  }
  template {
      disable_file_sandbox = true
  }
}

plugin "docker" {
  config {
    endpoint = "$docker_endpoint"
    gc {
        image = false
    }
    tls {
      cert = "/etc/nomad/cert.pem"
      key  = "/etc/nomad/key.pem"
      ca   = "/etc/nomad/ca.pem"
    }
    volumes {
        enabled = true
    }
    allow_privileged = true

  }
}

plugin "raw_exec" {
  config {
    enabled = true
  }
}

telemetry {
  collection_interval = "1s"
  disable_hostname = true
  prometheus_metrics = true
  publish_allocation_metrics = true
  publish_node_metrics = true
}


