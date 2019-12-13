resource "nomad_job" "stress_cpu" {
  jobspec = "${data.template_file.stress_cpu_template.rendered}"
}

resource "nomad_job" "stress_memory" {
  jobspec = "${data.template_file.stress_memory_template.rendered}"
}
