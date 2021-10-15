nomad stop -purge nginx
nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
nomad stop -purge discovery
nomad stop -purge broker
nomad stop -purge receiver
nomad stop -purge authorizer
nomad stop -purge file_transfer
