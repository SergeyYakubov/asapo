variable "elk_logs" {}

variable "perf_monitor" {}

variable "asapo_monitor" {}

variable "asapo_monitor_alert" {}

variable "asapo_alert_email" {}

variable "asapo_alert_email_smart_host" {}

variable "force_pull_images" {}

variable "asapo_user" {}

variable "nomad_logs" {}

variable "nginx_version" {}

variable "grafana_version" {}

variable "elasticsearch_version" {}

variable "kibana_version" {}

variable "influxdb_version" {}

variable "mongo_version" {}

variable "prometheus_version" {}

variable "alertmanager_version" {}

variable "asapo_imagename_suffix" {}

variable "asapo_image_tag" {}

variable "job_scripts_dir" {}

variable "service_dir" {}

variable "online_dir" {}

variable "offline_dir" {}

variable "mongo_dir" {}

variable "receiver_total_memory_size" {}

variable "receiver_dataserver_cache_size" {}

variable "receiver_dataserver_nthreads" {}

variable "receiver_receive_to_disk_threshold" {}

variable "receiver_network_modes" {}

variable "receiver_expose_metrics" {}

variable "grafana_total_memory_size" {}

variable "influxdb_total_memory_size" {}

variable "fluentd_total_memory_size" {}

variable "elasticsearch_total_memory_size" {}

variable "kibana_total_memory_size" {}

variable "mongo_total_memory_size" {}

variable "authorizer_total_memory_size" {}

variable "discovery_total_memory_size" {}

variable "prometheus_total_memory_size" {}

variable "alertmanager_total_memory_size" {}


variable "grafana_port" {}

variable "influxdb_port" {}

variable "influxdb_rpc_port" {}

variable "prometheus_port" {}

variable "alertmanager_port" {}

variable "mongo_port" {}

variable "fluentd_port" {}

variable "fluentd_port_stream" {}

variable "elasticsearch_port" {}

variable "kibana_port" {}

variable "discovery_port" {}

variable "authorizer_port" {}

variable "consul_dns_port" {}

variable "n_receivers" {}

variable "n_brokers" {}

variable "n_fts" {}

variable "ldap_uri" {}