worker_processes  1;

events {
    worker_connections  10000;
}

error_log         "/dev/stdout";
pid               "/tmp/nginx.pid";

http {
#    include       mime.types;
#    default_type  application/octet-stream;

#    sendfile        on;
#    tcp_nopush     on;

#    keepalive_timeout  0;
#    keepalive_timeout  65;

	 access_log off;


    client_body_temp_path  "/tmp/client_body" 1 2;
    proxy_temp_path        "/tmp/proxy" 1 2;
    fastcgi_temp_path      "/tmp/fastcgi" 1 2;
    scgi_temp_path         "/tmp/scgi" 1 2;
    uwsgi_temp_path        "/tmp/uwsgi" 1 2;


    resolver 127.0.0.1:{{ env "NOMAD_META_consul_dns_port" }} valid=1s;
    server {
          listen {{ env "NOMAD_PORT_nginx" }} reuseport;
          set $grafana_endpoint grafana.service.asapo;
          set $influxdb_endpoint influxdb.service.asapo;

   		  location /influxdb/ {
            rewrite ^/influxdb(/.*) $1 break;
            proxy_pass http://$influxdb_endpoint:{{ env "NOMAD_META_influxdb_port" }}$uri$is_args$args;
          }

           location /monitoring/ {
            rewrite ^/monitoring(/.*) $1 break;
            proxy_pass http://$grafana_endpoint:{{ env "NOMAD_META_grafana_port" }}$uri$is_args$args;
          }

      	  location /nginx-health {
  	        return 200 "healthy\n";
	      }
    }
}

stream {
    resolver 127.0.0.1:{{ env "NOMAD_META_consul_dns_port" }} valid=1s;

    map $remote_addr $upstream {
        default telegraf.service.asapo;
    }

    server {
        listen {{ env "NOMAD_PORT_nginx_stream" }} udp;
        proxy_pass $upstream:{{ env "NOMAD_META_telegraf_port_stream" }};
    }
}
