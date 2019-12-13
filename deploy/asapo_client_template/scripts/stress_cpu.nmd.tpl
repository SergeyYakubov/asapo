job "stress-cpu" {
  datacenters = ["dc1"]

  type = "system"

  group "stress-cpu" {
    count = 1

    task "stress-cpu" {
      driver = "docker"
      user = "${asapo_user}"

      config {
        network_mode = "host"
	    security_opt = ["no-new-privileges"]
	    userns_mode = "host"
        image = "progrium/stress"
        args = ["--cpu","${n_cpus}"]
        volumes = [
          "${data_dir}:/data"
        ]
      }

      resources {
        cpu    = 500
      }

	  service {
        name = "stress-cpu"
        check {
          name     = "stress-cpu-alive"
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
