resource "null_resource" "nginx" {
  provisioner "local-exec" {
    command = "asapo-wait-service nginx"
  }
  depends_on = [nomad_job.asapo-nginx]

}

resource "null_resource" "influxdb" {
  provisioner "local-exec" {
    command = "asapo-wait-service influxdb ${var.perf_monitor}"
  }
  depends_on = [nomad_job.asapo-perfmetrics]
}

resource "null_resource" "fluentd" {
  provisioner "local-exec" {
    command = "asapo-wait-service fluentd ${! var.nomad_logs}"
  }
  depends_on = [nomad_job.asapo-logging,null_resource.elasticsearch]
}

resource "null_resource" "mongo" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-mongodb"
  }
  depends_on = [nomad_job.asapo-mongo]

}

resource "null_resource" "asapo-authorizer" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-authorizer"
  }
  depends_on = [nomad_job.asapo-services]

}

resource "null_resource" "asapo-discovery" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-discovery"
  }
  depends_on = [nomad_job.asapo-services]
}


resource "null_resource" "asapo-broker" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-broker"
  }
  depends_on = [nomad_job.asapo-brokers]
}

resource "null_resource" "asapo-fts" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-file-transfer"
  }
  depends_on = [nomad_job.asapo-fts]
}

resource "null_resource" "asapo-receiver" {
  provisioner "local-exec" {
    command = "asapo-wait-service asapo-receiver"
  }
  depends_on = [nomad_job.asapo-receivers]
}

resource "null_resource" "elasticsearch" {
  provisioner "local-exec" {
    command = "asapo-wait-service elasticsearch ${var.elk_logs}"
  }
  depends_on = [nomad_job.asapo-logging]
}

resource "null_resource" "kibana" {
  provisioner "local-exec" {
    command = "asapo-wait-service kibana ${var.elk_logs}"
  }
  depends_on = [nomad_job.asapo-logging]
}


