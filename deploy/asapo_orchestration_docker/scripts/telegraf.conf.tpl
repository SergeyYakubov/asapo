[[inputs.statsd]]
  protocol = "udp"
  service_address = ":{{ env "NOMAD_PORT_telegraf_stream" }}"
  namepass= ["nomad_client_allocs"]
  templates = [
    "nomad.client.allocs.* measurement.measurement.measurement.field.field.job.group.alloc-id.task.hostname",
    "nomad.nomad.job_summary.* measurement.measurement.measurement.field.job.group.hostname"
  ]

[[inputs.consul]]

[[inputs.internal]]
  collect_memstats = false

[[outputs.file]]
	files=["stdout"]


[[outputs.influxdb]]
    urls = ["http://localhost:{{ env "NOMAD_META_nginx_port" }}/influxdb"]


[[outputs.health]]
  service_address = "http://{{ env "NOMAD_ADDR_telegraf_health" }}"

  namepass = ["internal_write"]
  tagpass = { output = ["influxdb"] }

  [[outputs.health.compares]]
    field = "buffer_size"
    lt = 5000.0

  [[outputs.health.contains]]
    field = "buffer_size"
