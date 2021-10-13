job "asapo-mongo" {
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

  group "mongo" {
    count = 1

    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    network {
      port "mongo" {
        static = "${mongo_port}"
      }
      port "mongo_monitor" {
        to = 9216
      }
    }

    task "mongo-monitor" {
      lifecycle {
        hook = "poststart"
        sidecar = true
      }

      driver = "docker"
      user = "${asapo_user}"

      config {
        security_opt = ["no-new-privileges"]
        userns_mode = "host"
        image = "yakser/mongodb-exporter"
        args = [
          "--mongodb.uri=mongodb://$${NOMAD_ADDR_mongo}"
        ]
        ports = ["mongo_monitor"]
      }

      service {
        port = "mongo_monitor"
        name = "asapo-mongodb-monitor"
        check {
          name = "alive"
          type     = "http"
          path     = "/"
          interval = "10s"
          timeout  = "1s"
        }
        check_restart {
          limit = 2
          grace = "6000s"
          ignore_warnings = false
        }
      }
    }

    task "mongo" {
      driver = "docker"
      user = "${asapo_user}"

      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "mongo:${mongo_version}"
        volumes = ["${mongo_dir}:/data/db"]
      }

      resources {
        memory = "${mongo_total_memory_size}"
      }

      service {
        port = "mongo"
        name = "asapo-mongodb"
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
          grace = "1800s"
          ignore_warnings = false
        }
      }
    }
  }
}
