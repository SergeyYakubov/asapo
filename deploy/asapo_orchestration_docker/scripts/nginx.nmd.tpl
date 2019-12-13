job "asapo-nginx" {
  datacenters = ["dc1"]

  type = "system"

  group "nginx" {
    count = 1

    restart {
      attempts = 2
      interval = "3m"
      delay = "16s"
      mode = "delay"
    }

    task "nginx" {
      driver = "docker"

      user = "${asapo_user}"

      meta {
        telegraf_port_stream = "${telegraf_port_stream}"
        grafana_port = "${grafana_port}"
        influxdb_port = "${influxdb_port}"
        consul_dns_port = "${consul_dns_port}"
      }

      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "nginx:${nginx_version}"
        volumes = [
          "local/nginx.conf:/etc/nginx/nginx.conf"
        ]
      }

      resources {
        cpu    = 500
        memory = ${nginx_total_memory_size}
        network {
          mbits = 10
          port "nginx" {
          	static = ${nginx_port}
          }
          port "nginx_stream" {
          	static = ${nginx_port_stream}
          }
        }
      }

      service {
        port = "nginx"
        name = "nginx"
        check {
          name     = "alive"
          type     = "http"
	      path     = "/nginx-health"
          timeout  = "2s"
	      interval = "10s"
        }

        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }
      }

      template {
         source        = "${scripts_dir}/nginx.conf.tpl"
         destination   = "local/nginx.conf"
         change_mode   = "restart"
      }
   }
  }
}
