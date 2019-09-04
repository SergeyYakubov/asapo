provider "nomad" {
  address = "http://localhost:4646"
}

variable "fluentd_logs" {
 default = true
}

variable "nginx_version" {
  default = "latest"
}

variable "asapo_imagename_suffix" {
  default = ""
}

variable "asapo_image_tag" {
  default = "latest"
}

variable "shared_dir" {
  default = "/tmp"
}

data "template_file" "nginx" {
  template = "${file("./asapo-nginx.nmd.tpl")}"
  vars = {
    nginx_version = "${var.nginx_version}"
  }
}

data "template_file" "asapo_services" {
  template = "${file("./asapo-services.nmd.tpl")}"
  vars = {
    image_suffix = "${var.asapo_imagename_suffix}:${var.asapo_image_tag}"
    shared_dir = "${var.shared_dir}"
    fluentd_logs = "${var.fluentd_logs}"
  }
}

resource "nomad_job" "asapo-nginx" {
  jobspec = "${data.template_file.nginx.rendered}"
}

resource "nomad_job" "asapo-services" {
  jobspec = "${data.template_file.asapo_services.rendered}"
}



