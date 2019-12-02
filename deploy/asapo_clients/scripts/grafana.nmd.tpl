job "grafana" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.asapo_service}"
    value     = "true"
    weight    = 100
  }

  group "grafana" {
    count = 1
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "grafana" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "grafana/grafana:${grafana_version}"
        volumes = ["/${service_dir}/grafana:/var/lib/grafana"]
      }

      resources {
        memory = "${grafana_total_memory_size}"
        network {
          port "grafana" {
          static = "${grafana_port}"
          }
        }
      }

     service {
       port = "grafana"
       name = "grafana"
       check {
           name = "alive"
           type     = "http"
           path     = "/api/health"
           interval = "10s"
           timeout  = "1s"
       }
       check_restart {
         limit = 2
         grace = "90s"
         ignore_warnings = false
       }
     }

   } #grafana
  }
}
