resource "null_resource" "nginx" {
  provisioner "local-exec" {
    command = "asapo-wait-service nginx"
  }
  depends_on = [nomad_job.nginx]
}

resource "null_resource" "influxdb" {
  provisioner "local-exec" {
    command = "asapo-wait-service influxdb"
  }
  depends_on = [nomad_job.monitoring]
}

resource "null_resource" "grafana" {
  provisioner "local-exec" {
    command = "asapo-wait-service grafana"
  }
  depends_on = [nomad_job.monitoring]
}


resource "null_resource" "telegraf" {
  provisioner "local-exec" {
    command = "asapo-wait-service telegraf"
  }
  depends_on = [nomad_job.monitoring]
}
