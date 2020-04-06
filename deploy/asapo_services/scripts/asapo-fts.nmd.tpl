job "asapo-file-transfer" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.node_group}"
    value     = "utl"
    weight    = 100
  }

  update {
    max_parallel = 1
    min_healthy_time = "10s"
    healthy_deadline = "3m"
    auto_revert = false
  }

  group "file-transfer" {
    count = ${n_file-transfer}
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "fail"
    }

    task "file-transfer" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "yakser/asapo-file-transfer${image_suffix}"
	    force_pull = true
        volumes = ["local/config.json:/var/lib/file_transfer/config.json",
                           "${offline_dir}:${offline_dir}",
                           "${online_dir}:${online_dir}"
                           ]
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
          port "file-transfer" {}
        }
      }

      service {
        port = "file-transfer"
        name = "asapo-file-transfer"
        check {
          name     = "asapo-file-transfer-alive"
          type     = "http"
          path     = "/health-check"
          interval = "10s"
          timeout  = "2s"
        }
        check_restart {
          limit = 2
          grace = "90s"
          ignore_warnings = false
        }
      }

      template {
         source        = "${scripts_dir}/file-transfer.json.tpl"
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
