job "asapo-mongo" {
  datacenters = ["dc1"]
  affinity {
    attribute = "${meta.asapo_service}"
    value     = "false"
    weight    = 100
  }
  update {
    max_parallel = 1
    min_healthy_time = "10s"
    healthy_deadline = "3m"
    auto_revert = false
  }

  group "mongo" {
    count = 1

    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "mongo" {
      driver = "docker"
      user = "${asapo_user}"

      config {
        network_mode = "host"
	    privileged = true
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "mongo:${mongo_version}"
        volumes = ["/${service_dir}/mongodb:/data/db"]
      }

      resources {
        memory = "${mongo_total_memory_size}"
        network {
          port "mongo" {
          static = "${mongo_port}"
          }
        }
      }

      service {
        port = "mongo"
        name = "mongo"
        check {
          type     = "script"
          name     = "alive"
          command  = "mongo"
          args     = ["--eval","db.version()"]
          interval = "10s"
          timeout  = "5s"
        }
        check_restart {
          limit = 2
          grace = "90s"
          ignore_warnings = false
        }
      }
    }
  }
}
