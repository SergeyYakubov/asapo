worker_processes  1;
daemon off;

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
        listen       {{ env "NOMAD_PORT_nginx" }};
          set $discovery_endpoint discovery.service.asapo;
          location /discovery/ {
            rewrite ^/discovery(/.*) $1 break;
            proxy_pass http://$discovery_endpoint:5006$uri;
        }
    }
}
