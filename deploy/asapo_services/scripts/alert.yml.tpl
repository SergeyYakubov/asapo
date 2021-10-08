groups:
- name: prometheus_alerts
  rules:
- name: asapo-nomad-alerts
  rules:
  - alert: asapo-services
    expr: sum(nomad_nomad_job_summary_running{exported_job="asapo-services"}) < 2 or absent(nomad_nomad_job_summary_running{exported_job="asapo-services"})
    for: 1s
    labels:
      severity: fatal
  - alert: asapo-monitoring
    expr: sum(nomad_nomad_job_summary_running{exported_job="asapo-monitoring"}) < 2  or absent(nomad_nomad_job_summary_running{exported_job="asapo-monitoring"})
    for: 1s
    labels:
      severity: fatal
  - alert: asapo-mongo
    expr: nomad_nomad_job_summary_running{exported_job="asapo-mongo"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-mongo"})
    for: 1s
    labels:
      severity: fatal
  - alert: asapo-receivers-incomplete
    expr: (nomad_nomad_job_summary_running{exported_job="asapo-receivers"} < {{ env "NOMAD_META_n_receivers" }} and sum (nomad_nomad_job_summary_running{exported_job="asapo-receivers"}) > 0)  or absent(nomad_nomad_job_summary_running{exported_job="asapo-receivers"})
    for: 30s
    labels:
      severity: warn
  - alert: asapo-receivers-absent
    expr: nomad_nomad_job_summary_running{exported_job="asapo-receivers"} < 1 or absent(nomad_nomad_job_summary_running{exported_job="asapo-receivers"})
    for: 1s
    labels:
      severity: fatal
  - alert: asapo-nginx
    expr: nomad_nomad_job_summary_running{exported_job="asapo-nginx"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-nginx"})
    for: 1s
    labels:
      severity: fatal
  - alert: asapo-fluentd
    expr: nomad_nomad_job_summary_running{exported_job="asapo-logging", task_group="fluentd"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-logging", task_group="fluentd"})
    for: 1s
    labels:
      severity: fatal

- name: asapo-consul-alerts
  rules:
    - alert: asapo-discovery
      expr: sum (up{job="asapo-discovery"}) < 1 or absent(up{job="asapo-discovery"})
      for: 1s
      labels:
        severity: fatal
    - alert: asapo-brokers-incomplete
      expr: (sum (up{job="asapo-broker"}) < {{ env "NOMAD_META_n_brokers" }} and sum (up{job="asapo-broker"}) > 0) or absent(up{job="asapo-broker"})
      for: 30s
      labels:
        severity: warn
    - alert: asapo-brokers-absent
      expr: sum (up{job="asapo-broker"}) == 0 or absent(up{job="asapo-broker"})
      for: 1s
      labels:
        severity: fatal