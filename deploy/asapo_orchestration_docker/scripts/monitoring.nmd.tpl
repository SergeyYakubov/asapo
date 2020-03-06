job "monitoring" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.asapo_service}"
    value     = "true"
    weight    = 100
  }

#  update {
#    max_parallel = 1
#    min_healthy_time = "10s"
#    healthy_deadline = "3m"
#    auto_revert = false
#  }

  group "monitoring" {
    count = "%{ if monitoring }1%{ else }0%{ endif }"
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

     meta {
          nginx_port = "${nginx_port}"
      }

    task "influxdb" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "bridge"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "influxdb:${influxdb_version}"
        volumes = [
          "/${service_dir}/influxdb:/var/lib/influxdb",
          "local/influxdb:/docker-entrypoint-initdb.d"]
        command = "influxd"
      }

      env {
        INFLUXDB_BIND_ADDRESS="127.0.0.1:$${NOMAD_PORT_influxdb_rpc}"
        INFLUXDB_HTTP_BIND_ADDRESS=":$${NOMAD_PORT_influxdb}"
      }

      resources {
        memory = "${influxdb_total_memory_size}"
        network {
          port "influxdb" {
          }
		  port "influxdb_rpc" {
          }
        }
      }

     service {
       port = "influxdb"
       name = "influxdb"
       check {
           name = "alive"
           type     = "http"
           path     = "/ping"
           interval = "10s"
           timeout  = "1s"
       }
       check_restart {
         limit = 2
         grace = "90s"
         ignore_warnings = false
       }
     }

    template {
      source = "${scripts_dir}/influxdb/create_db_logs.sh.tpl"
      destination = "local/influxdb/create_db_logs.sh"
    }
   } #influxdb


    task "grafana" {
      driver = "docker"
      user = "${asapo_user}"
      env {
        GF_SERVER_DOMAIN = "$${attr.unique.hostname}"
        GF_SERVER_ROOT_URL = "%(protocol)s://%(domain)s/monitoring/"
        GF_PATHS_PROVISIONING = "/var/lib/grafana_config/provisioning"
      }

      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "grafana/grafana:${grafana_version}"
        volumes = ["/${service_dir}/grafana:/var/lib/grafana",
        "local/grafana:/var/lib/grafana_config",
        "local/grafana/grafana.ini:/etc/grafana/grafana.ini"
        ]
      }

      resources {
        memory = "${grafana_total_memory_size}"
        network {
          port "grafana" {
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

      template {
         source        = "${scripts_dir}/grafana/grafana.ini.tpl"
         destination   = "local/grafana/grafana.ini"
         change_mode   = "restart"
      }
      template {
         source        = "${scripts_dir}/grafana/provisioning/dashboards/dashboards.yaml"
         destination   = "local/grafana/provisioning/dashboards/dashboards.yaml"
         change_mode   = "restart"
      }
      template {
         source        = "${scripts_dir}/grafana/provisioning/datasources/datasources.yaml.tpl"
         destination   = "local/grafana/provisioning/datasources/datasources.yaml"
         change_mode   = "restart"
      }
      template {
         source        = "${scripts_dir}/grafana/dashboards/grafana.json"
         destination   = "local/grafana/dashboards/grafana.json"
         change_mode   = "restart"
      }

   } #grafana

    task "telegraf" {
      driver = "docker"
      user = "${asapo_user}"
       config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "telegraf:${telegraf_version}"
        volumes = [
          "local/telegraf.conf:/etc/telegraf/telegraf.conf"
        ]
      }

      resources {
        memory = "${telegraf_total_memory_size}"
        network {
          port "telegraf_stream" {
          }
          port "telegraf_health" {
          }
        }
      }

	  service {
        name = "telegraf"
        port = "telegraf_stream"
        check {
          name     = "telegraf-alive"
          type     = "http"
          path     = "/"
          port     = "telegraf_health"
          interval = "10s"
          timeout  = "2s"
        }
#        check_restart {
#          limit = 2
#          grace = "15s"
#          ignore_warnings = false
#        }
      }

      template {
         source        = "${scripts_dir}/telegraf.conf.tpl"
         destination   = "local/telegraf.conf"
         change_mode   = "restart"
      }

   } #telegraf

  }
}
