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
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "yakser/asapo-receiver${image_suffix}"
	    force_pull = true
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
        }
          memory = "${receiver_total_memory_size}"
      }



      service {
        name = "asapo-receiver"
        port = "recv"
        check {
          name     = "asapo-receiver-alive"
          type     = "script"
          command  = "/bin/ps"
          args     = ["-fC","receiver"]
          interval = "10s"
          timeout  = "2s"
        }
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
      }

      template {
         source        = "${scripts_dir}/receiver.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
   } #task receivers
  }
}

