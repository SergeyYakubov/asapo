job "asapo-brokers" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.node_group}"
    value     = "blprx"
    weight    = 100
  }

  update {
    max_parallel = 1
    min_healthy_time = "10s"
    healthy_deadline = "3m"
    auto_revert = false
  }

  group "brokers" {
    count = ${n_brokers}
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "fail"
    }

    task "brokers" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "${image_registry}/asapo-broker${image_suffix}"
	    force_pull = ${force_pull_images}
        volumes = ["local/config.json:/var/lib/broker/config.json"]
        %{ if image_registry_username != "" }
        auth {
            username = "${image_registry_username}"
            password = "${image_registry_password}"
        }
        %{endif}
        %{ if ! nomad_logs  }
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
        network {
          port "broker" {}
        }
      }

      service {
        port = "broker"
        name = "asapo-broker"
        check {
          name     = "asapo-broker-alive"
          type     = "http"
          path     = "/health"
          interval = "10s"
          timeout  = "2s"
        }
        check_restart {
          limit = 2
          grace = "90s"
          ignore_warnings = false
        }
      }

      meta {
        perf_monitor = "${perf_monitor}"
      }

      template {
         source        = "${scripts_dir}/broker.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }

      template {
        source        = "${scripts_dir}/auth_secret.key"
        destination   = "local/secret.key"
        change_mode   = "restart"
      }
   } #task brokers
  }
}
