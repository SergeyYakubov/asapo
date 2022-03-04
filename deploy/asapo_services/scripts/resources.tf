resource "nomad_job" "asapo-nginx" {
  jobspec = data.template_file.nginx.rendered
}

resource "nomad_job" "asapo-mongo" {
  jobspec = data.template_file.asapo_mongo.rendered
}

resource "nomad_job" "asapo-perfmetrics" {
  jobspec = data.template_file.asapo_perfmetrics.rendered
}


resource "nomad_job" "asapo-monitoring" {
  jobspec = data.template_file.asapo_monitoring.rendered
}


resource "nomad_job" "asapo-logging" {
  jobspec = data.template_file.asapo_logging.rendered
  depends_on = [null_resource.nginx]
}

resource "nomad_job" "asapo-services" {
  jobspec = data.template_file.asapo_services.rendered
  depends_on = [null_resource.nginx,null_resource.mongo,null_resource.influxdb,null_resource.fluentd,null_resource.elasticsearch]
}

resource "nomad_job" "asapo-receivers" {
  jobspec = data.template_file.asapo_receivers.rendered
  depends_on = [nomad_job.asapo-services,null_resource.asapo-authorizer,null_resource.asapo-discovery,null_resource.asapo-monitoring-server]
}

resource "nomad_job" "asapo-brokers" {
  jobspec = data.template_file.asapo_brokers.rendered
  depends_on = [nomad_job.asapo-services,null_resource.asapo-authorizer,null_resource.asapo-discovery,null_resource.asapo-monitoring-server]
}

resource "nomad_job" "asapo-fts" {
  jobspec = data.template_file.asapo_fts.rendered
  depends_on = [nomad_job.asapo-services,null_resource.asapo-authorizer,null_resource.asapo-discovery,null_resource.asapo-monitoring-server]
}

