groups:
  - name: asapo-nomad-alerts
    rules:
      - alert: asapo-services
        expr: sum(nomad_nomad_job_summary_running{exported_job="asapo-services"}) < 2 or absent(nomad_nomad_job_summary_running{exported_job="asapo-services"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-monitoring
        expr: sum(nomad_nomad_job_summary_running{exported_job="asapo-monitoring"}) < 2  or absent(nomad_nomad_job_summary_running{exported_job="asapo-monitoring"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-mongo
        expr: nomad_nomad_job_summary_running{exported_job="asapo-mongo"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-mongo"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-receivers-incomplete
        expr: (nomad_nomad_job_summary_running{exported_job="asapo-receivers"} < {{ env "NOMAD_META_n_receivers" }} and sum (nomad_nomad_job_summary_running{exported_job="asapo-receivers"}) > 0)  or absent(nomad_nomad_job_summary_running{exported_job="asapo-receivers"})
        for: 60s
        labels:
          severity: warn
          group: asapo-cluster
      - alert: asapo-receivers-absent
        expr: nomad_nomad_job_summary_running{exported_job="asapo-receivers"} < 1 or absent(nomad_nomad_job_summary_running{exported_job="asapo-receivers"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-nginx
        expr: nomad_nomad_job_summary_running{exported_job="asapo-nginx"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-nginx"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-fts-incomplete
        expr: (nomad_nomad_job_summary_running{exported_job="asapo-file-transfer"} < {{ env "NOMAD_META_n_fts" }} and sum (nomad_nomad_job_summary_running{exported_job="asapo-file-transfer"}) > 0)  or absent(nomad_nomad_job_summary_running{exported_job="asapo-file-transfer"})
        for: 60s
        labels:
          severity: warn
          group: asapo-cluster
      - alert: asapo-fts-absent
        expr: nomad_nomad_job_summary_running{exported_job="asapo-file-transfer"} < 1 or absent(nomad_nomad_job_summary_running{exported_job="asapo-file-transfer"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-brokers-incomplete
        expr: (nomad_nomad_job_summary_running{exported_job="asapo-brokers"} < {{ env "NOMAD_META_n_brokers" }} and sum (nomad_nomad_job_summary_running{exported_job="asapo-brokers"}) > 0)  or absent(nomad_nomad_job_summary_running{exported_job="asapo-brokers"})
        for: 60s
        labels:
          severity: warn
          group: asapo-cluster
      - alert: asapo-brokers-absent
        expr: nomad_nomad_job_summary_running{exported_job="asapo-brokers"} < 1 or absent(nomad_nomad_job_summary_running{exported_job="asapo-brokers"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster          
      - alert: asapo-fluentd
        expr: nomad_nomad_job_summary_running{exported_job="asapo-logging", task_group="fluentd"} < 1  or absent(nomad_nomad_job_summary_running{exported_job="asapo-logging", task_group="fluentd"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster

  - name: asapo-consul-alerts
    rules:
      - alert: asapo-discovery
        expr: sum (up{job="asapo-discovery"}) < 1 or absent(up{job="asapo-discovery"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-mongodb-monitor
        expr: sum (up{job="asapo-mongodb-monitor"}) < 1 or absent(up{job="asapo-mongodb-monitor"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-brokers-incomplete
        expr: (sum (up{job="asapo-broker"}) < {{ env "NOMAD_META_n_brokers" }} and sum (up{job="asapo-broker"}) > 0) or absent(up{job="asapo-broker"})
        for: 60s
        labels:
          severity: warn
          group: asapo-cluster
      - alert: asapo-brokers-absent
        expr: sum (up{job="asapo-broker"}) == 0 or absent(up{job="asapo-broker"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
      - alert: asapo-receivers-incomplete
        expr: (sum (up{job="asapo-receiver"}) < {{ env "NOMAD_META_n_receivers" }} and sum (up{job="asapo-receiver"}) > 0) or absent(up{job="asapo-receiver"})
        for: 60s
        labels:
          severity: warn
          group: asapo-cluster
      - alert: asapo-receivers-absent
        expr: sum (up{job="asapo-receiver"}) == 0 or absent(up{job="asapo-receiver"})
        for: 10s
        labels:
          severity: fatal
          group: asapo-cluster
