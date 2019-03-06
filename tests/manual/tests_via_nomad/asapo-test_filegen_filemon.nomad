job "asapo-test" {
  datacenters = [
    "dc1"]

  type = "batch"

  group "filegen-windows" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "windows"
    }

    count = 1

    task "filegen" {
      driver = "raw_exec"

      config {
        command = "local/filegen_win.exe"
        args = [
          "1",
          "10M",
          "10000",
          "120",
          "u:/asapo/test_folder/file_win"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/filegen_win.exe"
        mode = "file"
        destination = "local/filegen_win.exe"
      }
    }


  }
  #windows

  group "filegen-linux" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "linux"
    }

    constraint {
      attribute = "${meta.location}"
      value = "petra3"
    }

    count = 1

    task "filegen" {
      driver = "raw_exec"

      config {
        command = "local/filegen_linux"
        args = [
          "1",
          "10M",
          "10000",
          "120",
          "/run/user/data/file_lin"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/filegen_linux"
        mode = "file"
        destination = "local/filegen_linux"
      }
    }

  }


  group "worker-linux1" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "linux"
    }

    #    constraint {
    #      attribute = "${meta.location}"
    #      operator = "!="
    #      value = "petra3"
    #    }

    count = 1

    task "worker-linux" {
      driver = "raw_exec"

      config {
        command = "local/getnext_broker"
        args = [
          "psana002:8400",
          "asapo_test1",
          "16",
          "oTsKsj8i6WcW_gVzeIFvZCtSfMErjDELJEyAI23n7Ik=",
          "30000"]
      }

#      resources {
#        cpu = 5000
#        memory = 128
#        network {
#          mbits = 10000
#        }
#      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/getnext_broker-@ASAPO_VERSION@"
      }
    }

  }
  # worker-linux1


  group "worker-linux2" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "linux"
    }

    #    constraint {
    #      attribute = "${meta.location}"
    #      operator = "!="
    #      value = "petra3"
    #    }

    count = 1

    task "worker-linux" {
      driver = "raw_exec"

      config {
        command = "local/getnext_broker"
        args = [
          "psana002:8400",
          "asapo_test2",
          "16",
          "yzgAcLmijSLWIm8dBiGNCbc0i42u5HSm-zR6FRqo__Y=",
          "30000"]
      }
#      resources {
#        cpu = 5000
#        memory = 128
#        network {
#          mbits = 10000
#        }
#      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/getnext_broker-@ASAPO_VERSION@"
      }
    }

  }
  # worker-linux2

}
