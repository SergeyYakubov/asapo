variable "elk_logs" {
 default = true
}

variable "nginx_version" {
  default = "latest"
}

variable "grafana_version" {
  default = "latest"
}

variable "elasticsearch_version" {
  default = "latest"
}

variable "kibana_version" {
  default = "latest"
}

variable "influxdb_version" {
  default = "latest"
}

variable "mongo_version" {
  default = "latest"
}

variable "asapo_imagename_suffix" {
  default = ""
}

variable "asapo_image_tag" {
#  default = "latest"
}

variable "job_scripts_dir" {
  default = "/var/run/asapo"
}

variable "service_dir" {
}

variable "data_dir" {
}

variable "receiver_total_memory_size" {
  default = "2000" #mb
}

variable "receiver_dataserver_cache_size" {
  default = "1"
}

variable "grafana_total_memory_size" {
  default = "256" #mb
}

variable "influxdb_total_memory_size" {
  default = "256" #mb
}

variable "fluentd_total_memory_size" {
  default = "256"
}

variable "elasticsearch_total_memory_size" {
  default = "256"
}

variable "kibana_total_memory_size" {
  default = "256"
}

variable "mongo_total_memory_size" {
  default = "300"
}

variable "authorizer_total_memory_size" {
  default = "256"
}

variable "discovery_total_memory_size" {
  default = "256"
}

variable "grafana_port" {
  default = "3000"
}

variable "influxdb_port" {
  default = "8086"
}

variable "mongo_port" {
  default = "27017"
}

variable "fluentd_port" {
  default = "9880"
}

variable "fluentd_port_stream" {
  default = "24224"
}

variable "elasticsearch_port" {
  default = "9200"
}

variable "kibana_port" {
  default = "5601"
}

variable "discovery_port" {
  default = "5006"
}

variable "authorizer_port" {
  default = "5007"
}
