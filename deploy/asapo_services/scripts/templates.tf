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
    prometheus_port = "${var.prometheus_port}"
    authorizer_port = "${var.authorizer_port}"
    discovery_port = "${var.discovery_port}"
    asapo_user = "${var.asapo_user}"
    consul_dns_port = "${var.consul_dns_port}"
    prometheus_port = "${var.prometheus_port}"
    alertmanager_port = "${var.alertmanager_port}"
    monitoring_ui_port = "${var.monitoring_ui_port}"
  }
}

data "template_file" "asapo_services" {
  template = "${file("${var.job_scripts_dir}/asapo-services.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    online_dir = "${var.online_dir}"
    offline_dir = "${var.offline_dir}"
    docker_repository = "${var.asapo_docker_repository}"
    ldap_uri = "${var.ldap_uri}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    authorizer_total_memory_size = "${var.authorizer_total_memory_size}"
    discovery_total_memory_size = "${var.discovery_total_memory_size}"
    authorizer_port = "${var.authorizer_port}"
    discovery_port = "${var.discovery_port}"
    asapo_user = "${var.asapo_user}"
    force_pull_images = "${var.force_pull_images}"
  }
}

data "template_file" "asapo_receivers" {
  template = "${file("${var.job_scripts_dir}/asapo-receivers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    online_dir = "${var.online_dir}"
    offline_dir = "${var.offline_dir}"
    docker_repository = "${var.asapo_docker_repository}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    receiver_total_memory_size = "${var.receiver_total_memory_size}"
    receiver_dataserver_cache_size = "${var.receiver_dataserver_cache_size}"
    receiver_receive_to_disk_threshold= "${var.receiver_receive_to_disk_threshold}"
    receiver_dataserver_nthreads = "${var.receiver_dataserver_nthreads}"
    receiver_network_modes = "${var.receiver_network_modes}"
    asapo_user = "${var.asapo_user}"
    n_receivers = "${var.n_receivers}"
    force_pull_images = "${var.force_pull_images}"
    perf_monitor = "${var.perf_monitor}"
    receiver_expose_metrics = "${var.receiver_expose_metrics}"
    receiver_kafka_enabled = "${var.receiver_kafka_enabled}"
    receiver_kafka_metadata_broker_list = "${var.receiver_kafka_metadata_broker_list}"
  }
}

data "template_file" "asapo_brokers" {
  template = "${file("${var.job_scripts_dir}/asapo-brokers.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    docker_repository = "${var.asapo_docker_repository}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    asapo_user = "${var.asapo_user}"
    n_brokers = "${var.n_brokers}"
    force_pull_images = "${var.force_pull_images}"
    perf_monitor = "${var.perf_monitor}"
  }
}


data "template_file" "asapo_fts" {
  template = "${file("${var.job_scripts_dir}/asapo-fts.nmd.tpl")}"
  vars = {
    scripts_dir = "${var.job_scripts_dir}"
    online_dir = "${var.online_dir}"
    offline_dir = "${var.offline_dir}"
    docker_repository = "${var.asapo_docker_repository}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    nomad_logs = "${var.nomad_logs}"
    asapo_user = "${var.asapo_user}"
    n_fts = "${var.n_fts}"
    force_pull_images = "${var.force_pull_images}"
  }
}

data "template_file" "asapo_perfmetrics" {
  template = "${file("${var.job_scripts_dir}/asapo-perfmetrics.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    docker_repository = "${var.asapo_docker_repository}"
    influxdb_version = "${var.influxdb_version}"
    grafana_version = "${var.grafana_version}"
    grafana_total_memory_size = "${var.grafana_total_memory_size}"
    grafana_port = "${var.grafana_port}"
    influxdb_total_memory_size = "${var.influxdb_total_memory_size}"
    influxdb_port = "${var.influxdb_port}"
    asapo_user = "${var.asapo_user}"
    influxdb_rpc_port = "${var.influxdb_rpc_port}"
    perf_monitor = "${var.perf_monitor}"
    monitoring_server_total_memory_size = "${var.monitoring_server_total_memory_size}"
    monitoring_proxy_total_memory_size = "${var.monitoring_proxy_total_memory_size}"
    monitoring_ui_total_memory_size = "${var.monitoring_ui_total_memory_size}"
    monitoring_proxy_port = "${var.monitoring_proxy_port}"
    monitoring_ui_port = "${var.monitoring_ui_port}"
    force_pull_images = "${var.force_pull_images}"
    nomad_logs = "${var.nomad_logs}"
    scripts_dir = "${var.job_scripts_dir}"
  }
}


data "template_file" "asapo_monitoring" {
  template = "${file("${var.job_scripts_dir}/asapo-monitoring.nmd.tpl")}"
  vars = {
    n_brokers = "${var.n_brokers}"
    n_receivers = "${var.n_receivers}"
    n_fts = "${var.n_fts}"
    service_dir = "${var.service_dir}"
    scripts_dir = "${var.job_scripts_dir}"
    asapo_monitor = "${var.asapo_monitor}"
    asapo_monitor_alert = "${var.asapo_monitor_alert}"
    prometheus_version = "${var.prometheus_version}"
    alertmanager_version = "${var.alertmanager_version}"
    alertmanager_port = "${var.alertmanager_port}"
    prometheus_port = "${var.prometheus_port}"
    prometheus_total_memory_size = "${var.prometheus_total_memory_size}"
    alertmanager_total_memory_size = "${var.alertmanager_total_memory_size}"
    asapo_user = "${var.asapo_user}"
    asapo_alert_email = "${var.asapo_alert_email}"
    asapo_alert_email_smart_host = "${var.asapo_alert_email_smart_host}"
  }
}

data "template_file" "asapo_mongo" {
  template = "${file("${var.job_scripts_dir}/asapo-mongo.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    mongo_dir = "${var.mongo_dir}"
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

