worker_processes  1;

events {
    worker_connections  100000;
}

error_log         "/dev/stdout";
pid               "/tmp/nginx.pid";

{{ $grafana := false -}}
{{ $influxdb := false -}}
{{ $telegraf := false -}}
{{ range service "grafana" -}} {{ $grafana = true }}{{ end -}}
{{ range service "influxdb" -}} {{ $influxdb = true }}{{ end -}}
{{ range service "telegraf" -}} {{ $telegraf = true }}{{ end -}}

http {
    access_log off;
    client_body_temp_path  "/tmp/client_body" 1 2;
    proxy_temp_path        "/tmp/proxy" 1 2;
    fastcgi_temp_path      "/tmp/fastcgi" 1 2;
    scgi_temp_path         "/tmp/scgi" 1 2;
    uwsgi_temp_path        "/tmp/uwsgi" 1 2;
{{ if $grafana }}
    upstream grafana {
    {{- range service "grafana" }}
      server {{ .Address }}:{{ .Port }};
    {{ end -}}
    }
{{ end -}}

{{- if $influxdb}}
    upstream influxdb {
    {{- range service "influxdb" }}
      server {{ .Address }}:{{ .Port }};
    {{ end -}}
    }
{{- end }}

    server {
          listen {{ env "NOMAD_PORT_nginx" }} reuseport;
{{- if $influxdb }}
   	  location /influxdb/ {
            rewrite ^/influxdb(/.*) $1 break;
            proxy_pass http://influxdb$uri$is_args$args;
          }
{{ end -}}
{{- if $grafana }}
           location /monitoring/ {
            rewrite ^/monitoring(/.*) $1 break;
            proxy_pass http://grafana$uri$is_args$args;
          }
{{- end }}

      	  location /nginx-health {
  	        return 200 "healthy\n";
	  }
    }
}

{{ if $telegraf -}}
stream {
    upstream telegraf {
	{{- range service "telegraf" }}
      server {{ .Address }}:{{ .Port }};
	{{- end }}
    }
    server {
        listen {{ env "NOMAD_PORT_nginx_stream" }} udp;
        proxy_pass telegraf;
    }
}
{{ end -}}


