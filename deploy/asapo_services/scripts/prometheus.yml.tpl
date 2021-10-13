# my global config
global:
  scrape_interval: 15s # Set the scrape interval to every 15 seconds. Default is every 1 minute.
  evaluation_interval: 15s # Evaluate rules every 15 seconds. The default is every 1 minute.
  # scrape_timeout is set to the global default (10s).

# Alertmanager configuration
alerting:
  alertmanagers:
    - consul_sd_configs:
        - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
          services: ['alertmanager']
      path_prefix: "alertmanager/"

# Load rules once and periodically evaluate them according to the global 'evaluation_interval'.
rule_files:
  - "alert.yml"
  # - "second_rules.yml"

# A scrape configuration containing exactly one endpoint to scrape:
# Here it's Prometheus itself.
scrape_configs:
  # The job name is added as a label `job=<job_name>` to any timeseries scraped from this config.
  - job_name: "prometheus"
    consul_sd_configs:
      - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
        services:
          - 'prometheus'
    relabel_configs:
      - source_labels: [__meta_consul_service]
        target_label: job
    metrics_path: /prometheus/metrics
  - job_name: "nomad metrics"
    consul_sd_configs:
      - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
        services:
          - 'nomad-client'
    metrics_path: /v1/metrics
    params:
      format: ['prometheus']
  - job_name: discovery
    consul_sd_configs:
      - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
        services:
          - 'asapo-discovery'
    relabel_configs:
      - source_labels: [__meta_consul_service]
        target_label: job
  - job_name: broker
    consul_sd_configs:
      - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
        services:
          - 'asapo-broker'
    relabel_configs:
      - source_labels: [__meta_consul_service]
        target_label: job
  - job_name: receiver
    consul_sd_configs:
      - server: '{{ env "NOMAD_IP_prometheus_ui" }}:8500'
        services:
          - 'asapo-receiver'
    relabel_configs:
      - source_labels: [__meta_consul_service]
        target_label: job
      - source_labels: [__meta_consul_service_address,__meta_consul_service_metadata_metrics_port]
        separator: ';'
        regex: (.*);(\d{4,5})
        target_label:  '__address__'
        replacement: '$1:$2'
        action: 'replace'


