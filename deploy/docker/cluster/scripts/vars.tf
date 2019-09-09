variable "fluentd_logs" {
 default = true
}

variable "nginx_version" {
  default = "latest"
}

variable "grafana_version" {
  default = "latest"
}


variable "influxdb_version" {
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
  default = "2000" #mb
}

variable "influxdb_total_memory_size" {
  default = "2000" #mb
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


variable "mongo_version" {
  default = "4.0.0"
}

variable "mongo_total_memory_size" {
  default = "300"
}

