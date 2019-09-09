data "template_file" "nginx" {
  template = "${file("${var.job_scripts_dir}/asapo-nginx.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    nginx_version = "${var.nginx_version}"
  }
}

data "template_file" "asapo_services" {
  template = "${file("${var.job_scripts_dir}/asapo-services.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    fluentd_logs = "${var.fluentd_logs}"
  }
}

data "template_file" "asapo_receivers" {
  template = "${file("${var.job_scripts_dir}/asapo-receivers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    data_dir = "${var.data_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    fluentd_logs = "${var.fluentd_logs}"
    receiver_total_memory_size = "${var.receiver_total_memory_size}"
    receiver_dataserver_cache_size = "${var.receiver_dataserver_cache_size}"
  }
}

data "template_file" "asapo_brokers" {
  template = "${file("${var.job_scripts_dir}/asapo-brokers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    fluentd_logs = "${var.fluentd_logs}"
  }
}


data "template_file" "asapo_perfmetrics" {
  template = "${file("${var.job_scripts_dir}/asapo-perfmetrics.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    influxdb_version = "${var.influxdb_version}"
    grafana_version = "${var.grafana_version}"
    grafana_total_memory_size = "${var.grafana_total_memory_size}"
    grafana_port = "${var.grafana_port}"
    influxdb_total_memory_size = "${var.influxdb_total_memory_size}"
    influxdb_port = "${var.influxdb_port}"

    }
}


data "template_file" "asapo_mongo" {
  template = "${file("${var.job_scripts_dir}/asapo-mongo.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    mongo_version = "${var.mongo_version}"
    mongo_total_memory_size = "${var.mongo_total_memory_size}"
    mongo_port = "${var.mongo_port}"
  }
}
