job "asapo-test" {
  datacenters = [
    "dc1"]

  type = "batch"

  group "producer-windows" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "windows"
    }

    count = 1
    task "producer" {
      driver = "raw_exec"

      config {
        command = "local/dummy-data-producer.exe"
        args = [
          "psana002:8400",
          "asapo_test1",
          "100",
          "1000000",
          "8",
          "0",
          "1000"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/dummy-data-producer-@ASAPO_VERSION@.exe"
        mode = "file"
        destination = "local/dummy-data-producer.exe"

      }

      #      resources {
      #        cpu = 5000
      #        memory = 128
      #        network {
      #          mbits = 10000
      #        }
    }

  }
  #windows

  group "producer-linux" {

    constraint {
      attribute = "${attr.kernel.name}"
      value = "linux"
    }

    constraint {
      attribute = "${meta.location}"
      value = "petra3"
    }

    count = 1

    task "producer" {
      driver = "raw_exec"

      config {
        command = "local/dummy-data-producer"
        args = [
          "psana002:8400",
          "asapo_test2",
          "100",
          "1000000",
          "8",
          "0",
          "1000"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/dummy-data-producer-@ASAPO_VERSION@"
      }

      #      resources {
      #        cpu = 5000
      #        memory = 128
      #        network {
      #          mbits = 10000
      #        }
      #      }

    }

  }
  #linux

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

      resources {
        cpu = 5000
        memory = 128
        network {
          mbits = 10000
        }
      }

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
      resources {
        cpu = 5000
        memory = 128
        network {
          mbits = 10000
        }
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/getnext_broker-@ASAPO_VERSION@"
      }
    }

  }

}
# worker-linux2
