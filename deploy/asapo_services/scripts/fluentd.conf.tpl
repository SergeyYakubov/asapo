<source>
  @type forward
  port {{ env "NOMAD_PORT_fluentd_stream" }}
  source_hostname_key source_addr
  bind 0.0.0.0
</source>

<source>
 @type http
 port {{ env "NOMAD_PORT_fluentd" }}
 bind 0.0.0.0
 add_remote_addr true
 format json
 time_format %Y-%m-%d %H:%M:%S.%N
</source>

<filter asapo.docker>
  @type parser
  key_name log
  format json
  time_format %Y-%m-%d %H:%M:%S.%N
  reserve_data true
</filter>

<filter asapo.docker>
  @type record_transformer
  enable_ruby
  remove_keys ["log","container_id","container_name"]
  <record>
   source_addr ${record["source_addr"].split('.')[0]}
 </record>
</filter>

<match asapo.**>
@type copy
{{ $use_logs := env "NOMAD_META_elk_logs" }}
{{ if eq $use_logs "true"}}
  <store>
  @type elasticsearch
  host localhost
  port 8400
  path /elasticsearch/
  logstash_format true
  time_key_format %Y-%m-%dT%H:%M:%S.%N
  time_key time
  time_key_exclude_timestamp true
  <buffer>
      @type memory
      total_limit_size 100MB
      flush_mode interval
      flush_interval 1s
      flush_thread_count 3
      chunk_limit_size 500KB
      overflow_action drop_oldest_chunk
  </buffer>
  </store>
{{ end }}
  <store>
  @type file
  flush_interval 1s
  append true
  buffer_type memory
  path /shared/asapo-logs
  </store>
</match>
