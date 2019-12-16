[[inputs.statsd]]
  protocol = "udp"
  service_address = ":{{ env "NOMAD_PORT_telegraf_stream" }}"
  namepass= ["nomad_client_allocs"]
  templates = [
    "nomad.client.allocs.* measurement.measurement.measurement.field.field.job.group.alloc-id.task.hostname",
    "nomad.nomad.job_summary.* measurement.measurement.measurement.field.job.group.hostname"
  ]

[[inputs.consul]]

[[outputs.file]]
	files=["stdout"]


[[outputs.influxdb]]
    urls = ["http://localhost:{{ env "NOMAD_META_nginx_port" }}/influxdb"]

