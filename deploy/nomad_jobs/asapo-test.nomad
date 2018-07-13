job "asapo-test" {
  datacenters = ["dc1"]

  type = "batch"

  group "windows-test" {

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
          "5000",
          "10000",
          "8",
          "0",
          "100"]
      }

      artifact {
        source = "https://stash.desy.de/rest/git-lfs/storage/ASAPO/asapo-bin/d42330e599f0197e8acc17fedb10c8ae897b57def38b207f1a58f14c8a524100?response-content-disposition=attachment%253B%2520filename%253D%2522dummy-data-producer.exe%2522%253B%2520filename*%253Dutf-8%27%27dummy-data-producer.exe"
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

  } #windows

  group "linux-test" {

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
          "5000",
          "10000",
          "8",
          "0",
          "100"]
      }

      artifact {
        source = "https://stash.desy.de/projects/ASAPO/repos/asapo-bin/raw/dummy-data-producer?at=refs%2Fheads%2Ffeature_ha"
      }

      #      resources {
      #        cpu = 5000
      #        memory = 128
      #        network {
      #          mbits = 10000
      #        }
      #      }

    }

  } #linux
}