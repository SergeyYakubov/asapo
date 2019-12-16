apiVersion: 1

datasources:
  - name: InfluxDB
    type: influxdb
    access: proxy
    database: telegraf
    url: http://localhost:{{ env "NOMAD_META_nginx_port" }}/influxdb
    jsonData:
      httpMode: GET
