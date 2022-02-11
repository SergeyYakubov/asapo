worker_processes  1;

events {
    worker_connections  1024;
}

error_log         "/tmp/nginx_error.log";
pid               "/tmp/nginx.pid";


http {
    sendfile            on;
    tcp_nopush          on;
    tcp_nodelay         on;
    keepalive_timeout   65;
    types_hash_max_size 2048;

    include             /etc/nginx/mime.types;
    default_type        application/octet-stream;

    client_body_temp_path  "/tmp/client_body" 1 2;
    client_max_body_size   10M;
    proxy_temp_path        "/tmp/proxy" 1 2;
    fastcgi_temp_path      "/tmp/fastcgi" 1 2;
    scgi_temp_path         "/tmp/scgi" 1 2;
    uwsgi_temp_path        "/tmp/uwsgi" 1 2;


    server {
        listen       {{ env "NOMAD_PORT_monitoring_ui" }} default_server;
        server_name  _;
        root         /usr/share/nginx/html;

        # Load configuration files for the default server block.
        #include /etc/nginx/default.d/*.conf;

        index index.html;

        etag on;

        location /js/ {
          add_header Cache-Control max-age=31536000;
        }

        location / {
          try_files $uri $uri/ /index.html;
        }
        location /index.html {
          add_header Cache-Control no-cache;
        }

    }
}