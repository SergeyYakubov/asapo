job "asapo-services" {
  datacenters = ["dc1"]

  type = "service"

  group "asapo-authorizer" {
    count = 1

    task "asapo-authorizer" {
      driver = "docker"

      config {
        network_mode = "host"
        dns_servers = ["127.0.0.1"]
        image = "yakser/asapo-authorizer${image_suffix}"
	force_pull = true
        volumes = ["local/config.json:/var/lib/authorizer/config.json"]
	%{ if elk_logs }
        logging {
            type = "fluentd"
            config {
                fluentd-address = "localhost:9881"
                fluentd-async-connect = true
                tag = "asapo.docker"
            }
        }
	%{endif}
      }

      resources {
        memory = "${authorizer_total_memory_size}"
        network {
          port "authorizer" {
            static = "${authorizer_port}"
          }
        }
      }

      service {
        name = "asapo-authorizer"
        port = "authorizer"
        check {
          name     = "alive"
          type     = "http"
          path     = "/health-check"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }
      }

      template {
         source        = "${scripts_dir}/authorizer.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
      template {
        source        = "${scripts_dir}/auth_secret.key"
        destination   = "local/secret.key"
        change_mode   = "restart"
      }
   }
  } #authorizer
  group "asapo-discovery" {
    count = 1

    task "asapo-discovery" {
      driver = "docker"

      config {
        network_mode = "host"
        dns_servers = ["127.0.0.1"]
        image = "yakser/asapo-discovery${image_suffix}"
	    force_pull = true
        volumes = ["local/config.json:/var/lib/discovery/config.json"]
        %{ if elk_logs }
        logging {
        type = "fluentd"
        config {
        fluentd-address = "localhost:9881"
        fluentd-async-connect = true
        tag = "asapo.docker"
        }
        }
        %{endif}
      }

      resources {
        memory = "${discovery_total_memory_size}"
        network {
          port "discovery" {
            static = "${discovery_port}"
          }
        }
      }

      service {
        name = "asapo-discovery"
        port = "discovery"
        check {
          name     = "alive"
          type     = "http"
          path     = "/receivers"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }

      }

      template {
         source        = "${scripts_dir}/discovery.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
   }
  }
}
