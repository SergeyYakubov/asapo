data "template_file" "grafana_template" {
  template = "${file("${var.job_scripts_dir}/grafana.nmd.tpl")}"
  vars = {
    service_dir = "${var.service_dir}"
    grafana_version = "${var.grafana_version}"
    grafana_total_memory_size = "${var.grafana_total_memory_size}"
    grafana_port = "${var.grafana_port}"
    asapo_user = "${var.asapo_user}"
    }
}


