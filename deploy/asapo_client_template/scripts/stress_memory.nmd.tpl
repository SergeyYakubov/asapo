job "stress-memory" {
  datacenters = ["dc1"]

  type = "system"

  group "stress-memory" {
    count = 1

    task "stress-memory" {
      driver = "docker"
      user = "${asapo_user}"

      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "progrium/stress"
        args = ["--vm","1","--vm-bytes","${alloc_size_mb}M","--vm-hang","30"]
      }

      resources {
        memory = ${total_memory_size}
      }

	  service {
        name = "stress-memory"
        check {
          name     = "stress-memory-alive"
          type     = "script"
          command  = "echo"
          args     = ["alive"]
          interval = "10s"
          timeout  = "2s"
        }
      }

   } #task
  } #group
} #job
