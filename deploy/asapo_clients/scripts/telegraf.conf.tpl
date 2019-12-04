[[inputs.statsd]]
  protocol = "udp"
  service_address = ":{{ env "NOMAD_PORT_telegraf_stream" }}"
  namepass= ["nomad_client_allocs*","nomad_nomad_job_summary*"]
  templates = [
    "nomad.client.allocs.* measurement.measurement.measurement.field.field.job.task-group.alloc-id.task.hostname",
    "nomad.nomad.job_summary.* measurement.measurement.measurement.field.job.task-group.hostname"
  ]

[[inputs.consul]]

[[outputs.influxdb]]
    urls = ["http://localhost:{{ env "NOMAD_META_nginx_port" }}/influxdb"]

