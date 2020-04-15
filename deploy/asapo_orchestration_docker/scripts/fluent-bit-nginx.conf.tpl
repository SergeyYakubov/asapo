[SERVICE]
    Parsers_File /etc/td-agent-bit/parsers.conf
    Parsers_File /var/run/asapo/fluent-bit-cluster-custom-parsers.conf

[INPUT]
    Name tail
    Path ${NOMAD_ALLOC_DIR}/${NOMAD_ALLOC_ID}/alloc/logs/nginx*.0
    Tag asapo.nginx
    DB ${NOMAD_TASK_DIR}/logs.db
    Parser level

[FILTER]
    Name modify
    Match *
    Condition Key_value_matches level (?i)^d.*
    Set level DEBUG

[FILTER]
    Name modify
    Match *
    Condition Key_value_matches level (?i)^(i.*|no.*)
    Set level INFO

[FILTER]
    Name modify
    Match *
    Condition Key_value_matches level (?i)^w.*
    Set level WARNING

[FILTER]
    Name modify
    Match *
    Condition Key_value_matches level (?i)^er.*
    Set level ERROR

[FILTER]
    Name modify
    Match *
    Condition Key_value_matches level (?i)^(c.*|a.*|emerg|exception)
    Set level CRITICAL

[FILTER]
    Name modify
    Match *
    Add level UNKNOWN

{{ range service "influxdb" }}
[OUTPUT]
    Name influxdb
    Match *
    Host {{ .Address }}
    Port {{ .Port }}
    Database logs
    Tag_Keys level
{{ end }}
