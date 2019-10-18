job "asapo-logging" {
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

  group "fluentd" {

    count = "%{ if nomad_logs }0%{ else }1%{ endif }"
    restart {
      attempts = 2
      interval = "3m"

      delay = "15s"
      mode = "delay"
    }

    task "fluentd" {
      driver = "docker"
      user = "${asapo_user}"
      meta {
        change_me_to_restart = 1
        elk_logs = "${elk_logs}"
      }
      config {
        network_mode = "host"
	    privileged = true
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "yakser/fluentd_elastic"
        volumes = ["local/fluentd.conf:/fluentd/etc/fluent.conf",
        "/${service_dir}/fluentd:/shared"]
      }

      resources {
        memory = "${fluentd_total_memory_size}"
        network {
          port "fluentd" {
          static = "${fluentd_port}"
          }
          port "fluentd_stream" {
            static = "${fluentd_port_stream}"
          }
        }
      }

      service {
        port = "fluentd"
        name = "fluentd"
        check {
          name     = "alive"
          type     = "script"
          command  = "/bin/pidof"
          args     = ["ruby"]
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
         source        = "${scripts_dir}/fluentd.conf.tpl"
         destination   = "local/fluentd.conf"
         change_mode   = "restart"
      }
   }
  }
#elasticsearch
  group "elk" {
    count = "%{ if elk_logs }1%{ else }0%{ endif }"
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "elasticsearch" {
      driver = "docker"
      user = "${asapo_user}"
      env {
        bootstrap.memory_lock = "true"
        cluster.name = "asapo-logging"
        ES_JAVA_OPTS = "-Xms512m -Xmx512m"
        discovery.type="single-node"
      }

      config {
        ulimit {
          memlock = "-1:-1"
          nofile = "65536:65536"
          nproc = "8192"
        }
        network_mode = "host"
	    privileged = true
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "yakser/elasticsearch:${elasticsearch_version}"
        volumes = ["/${service_dir}/esdatadir:/usr/share/elasticsearch/data"]
      }

      resources {
        memory = "${elasticsearch_total_memory_size}"
        network {
          port "elasticsearch" {
            static = "${elasticsearch_port}"
          }
         }
      }

      service {
        port = "elasticsearch"
        name = "elasticsearch"
        check {
            name = "alive"
            type     = "http"
	        path     = "/_cluster/health"
            interval = "10s"
            timeout  = "1s"
        }
        check_restart {
          limit = 2
          grace = "90s"
          ignore_warnings = false
        }
      }
   }
#kibana
   task "kibana" {
     driver = "docker"
     user = "${asapo_user}"
     config {
       network_mode = "host"
       privileged = true
	   security_opt = ["no-new-privileges"]
	   userns_mode = "host"
       image = "yakser/kibana:${kibana_version}"
       volumes = ["local/kibana.yml:/usr/share/kibana/config/kibana.yml"]
     }

      template {
         source        = "${scripts_dir}/kibana.yml"
         destination   = "local/kibana.yml"
         change_mode   = "restart"
      }

     resources {
       memory = "${kibana_total_memory_size}"
       network {
         port "kibana" {
           static = "${kibana_port}"
         }
        }
     }

     service {
       port = "kibana"
       name = "kibana"
       check {
           name = "alive"
           type     = "http"
           path     = "/logsview"
           interval = "10s"
           timeout  = "1s"
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
