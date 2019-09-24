worker_processes  1;

events {
    worker_connections  1024;
}

http {
#    include       mime.types;
#    default_type  application/octet-stream;

#    sendfile        on;
#    tcp_nopush     on;

#    keepalive_timeout  0;
#    keepalive_timeout  65;

    resolver 127.0.0.1:8600 valid=1s;
    server {
          listen {{ env "NOMAD_PORT_nginx" }};
          set $discovery_endpoint discovery.service.asapo;
          set $authorizer_endpoint authorizer.service.asapo;
          set $fluentd_endpoint fluentd.service.asapo;
          set $kibana_endpoint kibana.service.asapo;
          set $grafana_endpoint grafana.service.asapo;
          set $mongo_endpoint mongo.service.asapo;
          set $influxdb_endpoint influxdb.service.asapo;
          set $elasticsearch_endpoint elasticsearch.service.asapo;

   		  location /mongo/ {
            rewrite ^/mongo(/.*) $1 break;
            proxy_pass http://$mongo_endpoint:27017$uri$is_args$args;
          }

   		  location /influxdb/ {
            rewrite ^/influxdb(/.*) $1 break;
            proxy_pass http://$influxdb_endpoint:8086$uri$is_args$args;
          }

   		  location /elasticsearch/ {
            rewrite ^/elasticsearch(/.*) $1 break;
            proxy_pass http://$elasticsearch_endpoint:9200$uri$is_args$args;
          }

          location /discovery/ {
            rewrite ^/discovery(/.*) $1 break;
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

          location /authorizer/ {
             rewrite ^/authorizer(/.*) $1 break;
             proxy_pass http://$authorizer_endpoint:5007$uri$is_args$args;
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
        proxy_pass $upstream:24224;
    }
}

