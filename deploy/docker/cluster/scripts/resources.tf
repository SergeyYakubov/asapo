resource "nomad_job" "asapo-perfmetrics" {
  jobspec = "${data.template_file.asapo_perfmetrics.rendered}"
}

resource "nomad_job" "asapo-mongo" {
  jobspec = "${data.template_file.asapo_mongo.rendered}"
}

resource "nomad_job" "asapo-nginx" {
  jobspec = "${data.template_file.nginx.rendered}"
}

resource "nomad_job" "asapo-services" {
  jobspec = "${data.template_file.asapo_services.rendered}"
}

resource "nomad_job" "asapo-receivers" {
  jobspec = "${data.template_file.asapo_receivers.rendered}"
}

resource "nomad_job" "asapo-brokers" {
  jobspec = "${data.template_file.asapo_brokers.rendered}"
}
