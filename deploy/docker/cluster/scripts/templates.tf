data "template_file" "nginx" {
  template = "${file("${var.job_scripts_dir}/asapo-nginx.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    nginx_version = "${var.nginx_version}"
    fluentd_port = "${var.fluentd_port}"
    fluentd_port_stream = "${var.fluentd_port_stream}"
    kibana_port = "${var.kibana_port}"
    elasticsearch_port = "${var.elasticsearch_port}"
    grafana_port = "${var.grafana_port}"
    influxdb_port = "${var.influxdb_port}"
    authorizer_port = "${var.authorizer_port}"
    discovery_port = "${var.discovery_port}"
    asapo_user = "${var.asapo_user}"
  }
}

data "template_file" "asapo_services" {
  template = "${file("${var.job_scripts_dir}/asapo-services.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    authorizer_total_memory_size = "${var.authorizer_total_memory_size}"
    discovery_total_memory_size = "${var.discovery_total_memory_size}"
    authorizer_port = "${var.authorizer_port}"
    discovery_port = "${var.discovery_port}"
    asapo_user = "${var.asapo_user}"
  }
}

data "template_file" "asapo_receivers" {
  template = "${file("${var.job_scripts_dir}/asapo-receivers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    data_dir = "${var.data_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    receiver_total_memory_size = "${var.receiver_total_memory_size}"
    receiver_dataserver_cache_size = "${var.receiver_dataserver_cache_size}"
    receiver_dataserver_nthreads = "${var.receiver_dataserver_nthreads}"
    asapo_user = "${var.asapo_user}"
    n_receivers = "${var.n_receivers}"
  }
}

data "template_file" "asapo_brokers" {
  template = "${file("${var.job_scripts_dir}/asapo-brokers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    asapo_user = "${var.asapo_user}"
    n_brokers = "${var.n_brokers}"
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
    asapo_user = "${var.asapo_user}"
    }
}


data "template_file" "asapo_mongo" {
  template = "${file("${var.job_scripts_dir}/asapo-mongo.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    mongo_version = "${var.mongo_version}"
    mongo_total_memory_size = "${var.mongo_total_memory_size}"
    mongo_port = "${var.mongo_port}"
    asapo_user = "${var.asapo_user}"
  }
}

data "template_file" "asapo_logging" {
  template = "${file("${var.job_scripts_dir}/asapo-logging.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    scripts_dir = "${var.job_scripts_dir}"
    elk_logs = "${var.elk_logs}"
    nomad_logs = "${var.nomad_logs}"
    fluentd_total_memory_size = "${var.fluentd_total_memory_size}"
    fluentd_port = "${var.fluentd_port}"
    fluentd_port_stream = "${var.fluentd_port_stream}"
    kibana_version = "${var.kibana_version}"
    kibana_total_memory_size = "${var.kibana_total_memory_size}"
    kibana_port = "${var.kibana_port}"
    elasticsearch_version = "${var.elasticsearch_version}"
    elasticsearch_total_memory_size = "${var.elasticsearch_total_memory_size}"
    elasticsearch_port = "${var.elasticsearch_port}"
    asapo_user = "${var.asapo_user}"
  }
}

