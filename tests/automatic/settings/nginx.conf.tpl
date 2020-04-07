worker_processes  1;
daemon off;
events {
    worker_connections  1024;
}

error_log stderr info;
pid   "{{ env "NOMAD_ALLOC_DIR" }}/nginx.pid";

http {
	access_log off;
#    include       mime.types;
#    default_type  application/octet-stream;

#    sendfile        on;
#    tcp_nopush     on;

#    keepalive_timeout  0;
#    keepalive_timeout  65;

    client_body_temp_path  "{{ env "NOMAD_ALLOC_DIR" }}/tmp/client_body" 1 2;
    proxy_temp_path        "{{ env "NOMAD_ALLOC_DIR" }}/tmp/proxy" 1 2;
    fastcgi_temp_path      "{{ env "NOMAD_ALLOC_DIR" }}/tmp/fastcgi" 1 2;
    scgi_temp_path         "{{ env "NOMAD_ALLOC_DIR" }}/tmp/scgi" 1 2;
    uwsgi_temp_path        "{{ env "NOMAD_ALLOC_DIR" }}/tmp/uwsgi" 1 2;

    resolver 127.0.0.1:8600 valid=1s;
    server {
          listen {{ env "NOMAD_PORT_nginx" }};
          set $discovery_endpoint asapo-discovery.service.asapo;
          set $authorizer_endpoint asapo-authorizer.service.asapo;
          set $fluentd_endpoint fluentd.service.asapo;
          set $kibana_endpoint kibana.service.asapo;
          set $grafana_endpoint grafana.service.asapo;
          set $influxdb_endpoint influxdb.service.asapo;
          set $elasticsearch_endpoint elasticsearch.service.asapo;

   		  location /influxdb/ {
            rewrite ^/influxdb(/.*) $1 break;
            proxy_pass http://$influxdb_endpoint:8086$uri$is_args$args;
          }

   		  location /elasticsearch/ {
            rewrite ^/elasticsearch(/.*) $1 break;
            proxy_pass http://$elasticsearch_endpoint:9200$uri$is_args$args;
          }

          location /asapo-discovery/ {
            rewrite ^/asapo-discovery(/.*) $1 break;
            proxy_pass http://$discovery_endpoint:5006$uri$is_args$args;
          }

          location /logs/ {
              rewrite ^/logs(/.*) $1 break;
              proxy_pass http://$fluentd_endpoint:9880$uri$is_args$args;
          }

          location /logsview/ {
            proxy_pass http://$kibana_endpoint:5601$uri$is_args$args;
            proxy_set_header  X-Real-IP  $remote_addr;
            proxy_set_header  X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header  Host $http_host;
          }

          location /performance/ {
            rewrite ^/performance(/.*) $1 break;
            proxy_pass http://$grafana_endpoint:3000$uri$is_args$args;
          }

          location /asapo-authorizer/ {
             rewrite ^/asapo-authorizer(/.*) $1 break;
             proxy_pass http://$authorizer_endpoint:5007$uri$is_args$args;
          }

      	  location /nginx-health {
  	        return 200 "healthy\n";
	      }
    }
}

