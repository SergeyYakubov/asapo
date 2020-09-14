resource "nomad_job" "monitoring" {
  jobspec = "${data.template_file.monitoring_template.rendered}"
}

resource "nomad_job" "nginx" {
  jobspec = "${data.template_file.nginx_template.rendered}"
  depends_on = [nomad_job.monitoring,null_resource.influxdb,null_resource.grafana,null_resource.telegraf]
}
