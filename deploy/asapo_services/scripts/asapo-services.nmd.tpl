job "asapo-services" {
  datacenters = ["dc1"]
  affinity {
    attribute = "$${meta.node_group}"
    value     = "utl"
    weight    = 100
  }

  type = "service"

  group "asapo-authorizer" {
    count = 1

    task "asapo-authorizer" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "${docker_repository}/asapo-authorizer${image_suffix}"
	    force_pull = ${force_pull_images}
        volumes = ["local/config.json:/var/lib/authorizer/config.json",
                           "${offline_dir}:${offline_dir}",
                           "${online_dir}:${online_dir}"]

	%{ if ! nomad_logs }
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
        memory = "${authorizer_total_memory_size}"
        network {
          port "authorizer" {
            static = "${authorizer_port}"
          }
        }
      }

      meta {
        offline_dir = "${offline_dir}"
        online_dir = "${online_dir}"
      }

      service {
        name = "asapo-authorizer"
        port = "authorizer"
        check {
          name     = "alive"
          type     = "http"
          path     = "/health-check"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }
      }

      template {
         source        = "${scripts_dir}/authorizer.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
      template {
        source        = "${scripts_dir}/auth_secret.key"
        destination   = "local/secret.key"
        change_mode   = "restart"
      }
      template {
        source        = "${scripts_dir}/auth_secret_admin.key"
        destination   = "local/secret_admin.key"
        change_mode   = "restart"
      }
   }
  } #authorizer
  group "asapo-discovery" {
    count = 1

    task "asapo-discovery" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "${docker_repository}/asapo-discovery${image_suffix}"
	    force_pull = ${force_pull_images}
        volumes = ["local/config.json:/var/lib/discovery/config.json"]
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
        memory = "${discovery_total_memory_size}"
        network {
          port "discovery" {
            static = "${discovery_port}"
          }
        }
      }

      service {
        name = "asapo-discovery"
        port = "discovery"
        check {
          name     = "alive"
          type     = "http"
          path     = "/health"
          interval = "10s"
          timeout  = "2s"
          initial_status =   "passing"
        }
        check_restart {
          limit = 2
          grace = "15s"
          ignore_warnings = false
        }

      }

      template {
         source        = "${scripts_dir}/discovery.json.tpl"
         destination   = "local/config.json"
         change_mode   = "restart"
      }
   }
  }
}
