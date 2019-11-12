job "asapo-nginx" {
  datacenters = ["dc1"]

  type = "system"

#  update {
#    max_parallel = 1
#    min_healthy_time = "10s"
#    healthy_deadline = "3m"
#    auto_revert = false
#  }

  group "nginx" {
    count = 1

    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "nginx" {
      driver = "docker"

      user = "${asapo_user}"

      meta {
        fluentd_port = "${fluentd_port}"
        fluentd_port_stream = "${fluentd_port_stream}"
        kibana_port = "${kibana_port}"
        elasticsearch_port = "${elasticsearch_port}"
        grafana_port = "${grafana_port}"
        influxdb_port = "${influxdb_port}"
        authorizer_port = "${authorizer_port}"
        discovery_port = "${discovery_port}"
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
        memory = 256
        network {
          mbits = 10
          port "nginx" {
          static = 8400
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
