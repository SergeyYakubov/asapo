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
    network {
      port "monitoring_server" {}
    }

    task "influxdb" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "influxdb:${influxdb_version}"
        volumes = ["/${service_dir}/influxdb:/var/lib/influxdb"]
      }

      env {
        PRE_CREATE_DB="asapo_receivers;asapo_brokers"
        INFLUXDB_BIND_ADDRESS="127.0.0.1:$${NOMAD_PORT_influxdb_rpc}"
        INFLUXDB_HTTP_BIND_ADDRESS=":$${NOMAD_PORT_influxdb}"
        INFLUXDB_HTTP_FLUX_ENABLED="true"
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

    task "monitoring-server" {
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
        image = "${docker_repository}/asapo-monitoring-server${image_suffix}"
        force_pull = ${force_pull_images}
        volumes = ["local/config.json:/var/lib/monitoring_server/config.json"]
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
        memory = "${monitoring_server_total_memory_size}"
      }

      service {
        name = "asapo-monitoring"
        port = "monitoring_server"
        check {
          name     = "alive"
          port     = "monitoring_server"
          type     = "tcp"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
      }

      template {
        source        = "${scripts_dir}/monitoring_server.json.tpl"
        destination   = "local/config.json"
        change_mode   = "restart"
      }
    } # monitoring server

    task "monitoring-proxy" {
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
        image = "envoyproxy/envoy:v1.21.0"
        volumes = ["local/envoy.yaml:/etc/envoy/envoy.yaml"]
        command = "/usr/local/bin/envoy"
        args=["-c","/etc/envoy/envoy.yaml"] #,"-l","trace"
      }

      resources {
        memory = "${monitoring_proxy_total_memory_size}"
        network {
          port "monitoring_proxy_admin" {}
          port "monitoring_proxy" {
            static = "${monitoring_proxy_port}"
          }
        }
      }

      service {
        name = "asapo-monitoring-proxy"
        port = "monitoring_proxy"
        check {
          name     = "asapo-monitoring-proxy-alive"
          port     = "monitoring_proxy_admin"
          type     = "http"
          path     = "/"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
      }

      template {
        source        = "${scripts_dir}/monitoring_proxy.yaml.tpl"
        destination   = "local/envoy.yaml"
        change_mode   = "restart"
      }
      } # monitoring proxy

    task "monitoring-ui" {
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
        image = "${docker_repository}/asapo-monitoring-ui${image_suffix}"
        force_pull = ${force_pull_images}
        volumes = ["local/nginx.conf:/etc/nginx/nginx.conf"]
      }

      resources {
        memory = "${monitoring_ui_total_memory_size}"
        network {
          port "monitoring_ui" {
            static = "${monitoring_ui_port}"
          }
        }
      }

      service {
        name = "asapo-monitoring-ui"
        port = "monitoring_ui"
        check {
          name     = "asapo-monitoring-ui-alive"
          port     = "monitoring_ui"
          type     = "http"
          path     = "/"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
      }

      template {
        source        = "${scripts_dir}/monitoring_ui_nginx.conf.tpl"
        destination   = "local/nginx.conf"
        change_mode   = "restart"
      }
    } # monitoring ui


  }
}
