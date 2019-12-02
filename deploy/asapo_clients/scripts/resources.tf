resource "nomad_job" "grafana" {
  jobspec = "${data.template_file.grafana_template.rendered}"
}


