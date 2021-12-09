job "asapo-receivers" {
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

  group "receivers" {
    count = ${n_receivers}

    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "fail"
    }

    task "receivers" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        ulimit {
          memlock = "-1:-1"
        }
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
	    privileged = true
        image = "${docker_repository}/asapo-receiver${image_suffix}"
	    force_pull = ${force_pull_images}
        volumes = ["local/config.json:/var/lib/receiver/config.json",
                   "${offline_dir}:${offline_dir}",
                   "${online_dir}:${online_dir}"]
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
          port "recv" {}
          port "recv_ds" {}
          port "recv_metrics" {}
        }
          memory = "${receiver_total_memory_size}"
      }

      service {
        name = "asapo-receiver"
        port = "recv"
        %{ if receiver_expose_metrics  }
        check {
          name     = "metrics"
          type     = "http"
          port = "recv_metrics"
          path     = "/metrics"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
        meta {
          metrics-port = "$${NOMAD_PORT_recv_metrics}"
        }
      %{ else  }
       check {
          name     = "asapo-receiver-alive"
          type     = "script"
          command  = "/bin/ps"
          args     = ["-fC","receiver"]
          interval = "10s"
          timeout  = "2s"
        }
      %{ endif  }
        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }
      }

      meta {
        receiver_dataserver_cache_size = "${receiver_dataserver_cache_size}"
        receiver_dataserver_nthreads = "${receiver_dataserver_nthreads}"
        receiver_receive_to_disk_threshold = "${receiver_receive_to_disk_threshold}"
        receiver_network_modes = "${receiver_network_modes}"
        perf_monitor = "${perf_monitor}"
        receiver_expose_metrics = "${receiver_expose_metrics}"
      }

      template {
         source        = "${scripts_dir}/receiver.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
   } #task receivers
  }
}

