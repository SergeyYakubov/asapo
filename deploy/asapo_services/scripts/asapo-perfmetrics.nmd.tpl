job "asapo-perfmetrics" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.node_group}"
    value     = "utl"
    weight    = 100
  }

  group "perfmetrics" {
    count = "%{ if perf_monitor }1%{ else }0%{ endif }"
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "influxdb" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "influxdb:${influxdb_version}"
        volumes = ["/${service_dir}/influxdb2:/var/lib/influxdb2"]
      }

      env {
        PRE_CREATE_DB="asapo_receivers;asapo_brokers"
        INFLUXDB_BIND_ADDRESS="127.0.0.1:$${NOMAD_PORT_influxdb_rpc}"
        INFLUXDB_HTTP_BIND_ADDRESS=":$${NOMAD_PORT_influxdb}"
      }

      resources {
        memory = "${influxdb_total_memory_size}"
        network {
          port "influxdb" {
          static = "${influxdb_port}"
          }
          port "influxdb_rpc" {
          static = "${influxdb_rpc_port}"
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

   } #influxdb



    task "grafana" {
      driver = "docker"
      user = "${asapo_user}"
      env {
        GF_SERVER_DOMAIN = "$${attr.unique.hostname}"
        GF_SERVER_ROOT_URL = "%(protocol)s://%(domain)s/performance/"
        GF_SERVER_HTTP_PORT = "${grafana_port}"
      }

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
