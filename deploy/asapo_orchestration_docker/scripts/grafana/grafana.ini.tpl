[server]
http_port = {{ env "NOMAD_PORT_grafana" }}

[auth.anonymous]
enabled = true
org_name = Main Org.
org_role = Admin

#[auth]
#disable_login_form = true
