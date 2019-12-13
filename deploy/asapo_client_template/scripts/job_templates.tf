data "template_file" "stress_cpu_template" {
  template = "${file("${var.job_scripts_dir}/stress_cpu.nmd.tpl")}"
  vars = {
	asapo_user = "${var.asapo_user}"
    data_dir = "${var.data_dir}"
    n_cpus = "${var.stress_cpu_n_cpus}"
    }
}

data "template_file" "stress_memory_template" {
  template = "${file("${var.job_scripts_dir}/stress_memory.nmd.tpl")}"
  vars = {
    asapo_user = "${var.asapo_user}"
    total_memory_size = "${var.stress_memory_total_memory_size}"
    alloc_size_mb = "${var.stress_memory_alloc_size_mb}"
  }
}

