c:\opt\consul\nomad stop -purge receiver
c:\opt\consul\nomad stop -purge discovery
c:\opt\consul\nomad stop -purge broker
c:\opt\consul\nomad stop -purge authorizer
c:\opt\consul\nomad stop -purge nginx
c:\opt\consul\nomad stop -purge file_transfer
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
