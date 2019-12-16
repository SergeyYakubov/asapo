data "template_file" "monitoring_template" {
  template = "${file("${var.job_scripts_dir}/monitoring.nmd.tpl")}"
  vars = {
  	monitoring = "${var.monitoring}"
    service_dir = "${var.service_dir}"
    grafana_version = "${var.grafana_version}"
    grafana_total_memory_size = "${var.grafana_total_memory_size}"
    grafana_port = "${var.grafana_port}"
    grafana_version = "${var.grafana_version}"
    grafana_total_memory_size = "${var.grafana_total_memory_size}"
    grafana_port = "${var.grafana_port}"
    influxdb_version = "${var.influxdb_version}"
    influxdb_total_memory_size = "${var.influxdb_total_memory_size}"
    influxdb_port = "${var.influxdb_port}"
    telegraf_version = "${var.telegraf_version}"
    telegraf_total_memory_size = "${var.telegraf_total_memory_size}"
    telegraf_port_stream = "${var.telegraf_port_stream}"
    nginx_port = "${var.nginx_port}"
    asapo_user = "${var.asapo_user}"
    scripts_dir = "${var.job_scripts_dir}"
    }
}

data "template_file" "nginx_template" {
  template = "${file("${var.job_scripts_dir}/nginx.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    nginx_total_memory_size = "${var.nginx_total_memory_size}"
    nginx_version = "${var.nginx_version}"
    nginx_port = "${var.nginx_port}"
    nginx_port_stream = "${var.nginx_port_stream}"
    telegraf_port_stream = "${var.telegraf_port_stream}"
    grafana_port = "${var.grafana_port}"
    influxdb_port = "${var.influxdb_port}"
    asapo_user = "${var.asapo_user}"
    consul_dns_port = "${var.consul_dns_port}"
  }
}

