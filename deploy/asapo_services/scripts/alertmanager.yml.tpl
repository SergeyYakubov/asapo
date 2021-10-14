route:
  group_by: ['group','severity']
  group_wait: 30s
  group_interval: 5m
  repeat_interval: 1h
  receiver: 'email'
receivers:
  - name: 'email'
    email_configs:
      - smarthost: '{{ env "NOMAD_META_email_smart_host" }}'
        to: '{{ env "NOMAD_META_alert_email" }}'
        from: 'noreply@desy.de'
        send_resolved: true
        require_tls: false
inhibit_rules:
  - source_match:
      severity: 'critical'
    target_match:
      severity: 'warning'
    equal: ['alertname', 'dev', 'instance']
