worker_processes  1;

events {
    worker_connections  1024;
}

error_log         "/tmp/nginx_error.log";
pid               "/tmp/nginx.pid";

http {
#    include       mime.types;
#    default_type  application/octet-stream;

#    sendfile        on;
#    tcp_nopush     on;

#    keepalive_timeout  0;
#    keepalive_timeout  65;


    client_body_temp_path  "/tmp/client_body" 1 2;
    proxy_temp_path        "/tmp/proxy" 1 2;
    fastcgi_temp_path      "/tmp/fastcgi" 1 2;
    scgi_temp_path         "/tmp/scgi" 1 2;
    uwsgi_temp_path        "/tmp/uwsgi" 1 2;


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
            proxy_pass http://$influxdb_endpoint:{{ env "NOMAD_META_influxdb_port" }}$uri$is_args$args;
          }

   		  location /elasticsearch/ {
            rewrite ^/elasticsearch(/.*) $1 break;
            proxy_pass http://$elasticsearch_endpoint:{{ env "NOMAD_META_elasticsearch_port" }}$uri$is_args$args;
          }

          location /discovery/ {
            rewrite ^/discovery(/.*) $1 break;
            proxy_pass http://$discovery_endpoint:{{ env "NOMAD_META_discovery_port" }}$uri$is_args$args;
          }

          location /logs/ {
              rewrite ^/logs(/.*) $1 break;
              proxy_pass http://$fluentd_endpoint:{{ env "NOMAD_META_fluentd_port" }}$uri$is_args$args;
          }

          location /logsview/ {
            proxy_pass http://$kibana_endpoint:{{ env "NOMAD_META_kibana_port" }}$uri$is_args$args;
            proxy_set_header  X-Real-IP  $remote_addr;
            proxy_set_header  X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header  Host $http_host;
          }

          location /performance/ {
            rewrite ^/performance(/.*) $1 break;
            proxy_pass http://$grafana_endpoint:{{ env "NOMAD_META_grafana_port" }}$uri$is_args$args;
          }

          location /authorizer/ {
             rewrite ^/authorizer(/.*) $1 break;
             proxy_pass http://$authorizer_endpoint:{{ env "NOMAD_META_authorizer_port" }}$uri$is_args$args;
          }

      	  location /nginx-health {
  	        return 200 "healthy\n";
	      }
    }
}

stream {
    resolver 127.0.0.1:8600 valid=1s;

    map $remote_addr $upstream {
        default fluentd.service.asapo;
    }

    server {
        listen     9881;
        proxy_pass $upstream:{{ env "NOMAD_META_fluentd_port_stream" }};
    }
}
