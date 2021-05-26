nomad stop nginx
nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
nomad stop discovery
nomad stop broker
nomad stop receiver
nomad stop authorizer
nomad stop file_transfer
