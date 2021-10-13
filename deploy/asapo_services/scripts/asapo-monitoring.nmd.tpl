job "asapo-monitoring" {
  datacenters = [
    "dc1"]
  affinity {
    attribute = "$${meta.node_group}"
    value = "utl"
    weight = 100
  }

  #  update {
  #    max_parallel = 1
  #    min_healthy_time = "10s"
  #    healthy_deadline = "3m"
  #    auto_revert = false
  #  }

  group "alerting" {
    count = "%{ if asapo_monitor_alert }1%{ else }0%{ endif }"
    restart {
      attempts = 2
      interval = "30m"
      delay = "15s"
      mode = "fail"
    }

    task "alertmanager" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        image = "prom/alertmanager:${alertmanager_version}"
        args = [
          "--web.route-prefix=/alertmanager/",
          "--config.file=/etc/alertmanager/alertmanager.yml",
          "--storage.path=/alertmanager"
        ]
        volumes = [
          "/${service_dir}/alertmanager:/alertmanager",
          "local/alertmanager.yml:/etc/alertmanager/alertmanager.yml",
        ]
      }
      template {
        source = "${scripts_dir}/alertmanager.yml.tpl"
        destination = "local/alertmanager.yml"
        change_mode = "restart"
      }
      resources {
        memory = "${alertmanager_total_memory_size}"
        network {
          mbits = 10
          port "alertmanager_ui" {
            static = "${alertmanager_port}"
            to = 9093
          }
        }
      }

      meta {
        alert_email = "${asapo_alert_email}"
        email_smart_host = "${asapo_alert_email_smart_host}"
      }

      service {
        name = "alertmanager"
        port = "alertmanager_ui"
        check {
          name = "alertmanager_ui port alive"
          type = "http"
          path = "/alertmanager/-/healthy"
          interval = "10s"
          timeout = "2s"
        }
      }
    }
  }


  group "monitoring" {
    count = "%{ if asapo_monitor }1%{ else }0%{ endif }"
    restart {
      attempts = 2
      interval = "3m"
      delay = "15s"
      mode = "delay"
    }

    task "prometheus" {
      driver = "docker"
      user = "${asapo_user}"
      config {
        image = "prom/prometheus:${prometheus_version}"
        args = [
          "--web.external-url=/prometheus/",
          "--web.route-prefix=/prometheus/",
          "--config.file=/etc/prometheus/prometheus.yml"
        ]
        volumes = [
          "local/alert.yml:/etc/prometheus/alert.yml",
          "local/prometheus.yml:/etc/prometheus/prometheus.yml",
          "/${service_dir}/prometheus:/prometheus"
        ]
      }
      template {
        source = "${scripts_dir}/alert.yml.tpl"
        destination = "local/alert.yml"
        change_mode = "restart"
      }
      template {
        source = "${scripts_dir}/prometheus.yml.tpl"
        destination = "local/prometheus.yml"
        change_mode = "restart"
      }
      resources {
        memory = "${prometheus_total_memory_size}"
        network {
          mbits = 10
          port "prometheus_ui" {
            static = "${prometheus_port}"
            to = 9090
          }
        }
      }
      meta {
        n_brokers = "${n_brokers}"
        n_receivers = "${n_receivers}"
      }
      service {
        name = "prometheus"
        port = "prometheus_ui"
        check {
          name = "prometheus_ui port alive"
          type = "http"
          path = "/prometheus/-/healthy"
          interval = "10s"
          timeout = "2s"
        }
      }
    }
  }
}