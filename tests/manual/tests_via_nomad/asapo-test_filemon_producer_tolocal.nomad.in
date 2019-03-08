job "asapo-produceronly" {
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
        command = "local/asapo-eventmon-producer.exe"
        args = [
          "local/test.json"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/asapo-eventmon-producer-@ASAPO_VERSION@.exe"
        mode = "file"
        destination = "local/asapo-eventmon-producer.exe"
      }

      template {
        data = <<EOH
{
 "AsapoEndpoint":"c:\\tmp\\asapo\\test_out",
 "Tag":"test_tag",
 "BeamtimeID":"asapo_test",
 "Mode":"filesystem",
 "NThreads":1,
 "LogLevel":"debug",
 "RootMonitoredFolder":"c:\\tmp\\asapo\\test_in",
 "MonitoredSubFolders":["test_folder"],
 "IgnoreExtentions":["tmp"],
 "RemoveAfterSend":true
}
        EOH
        destination = "local/test.json"
      }
      #      resources {
      #        cpu = 5000
      #        memory = 128
      #        network {
      #          mbits = 10000
      #        }
    } # producer task
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
        command = "local/asapo-eventmon-producer"
        args = [
          "local/test.json"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/asapo-eventmon-producer-@ASAPO_VERSION@"
        mode = "file"
        destination = "local/asapo-eventmon-producer"
      }

      template {
        data = <<EOH
{
 "AsapoEndpoint":"/tmp/asapo/test_out",
 "Tag":"test_tag",
 "BeamtimeID":"asapo_test",
 "Mode":"filesystem",
 "NThreads":1,
 "LogLevel":"debug",
 "RootMonitoredFolder":"/tmp/asapo/test_in",
 "MonitoredSubFolders":["test_folder"],
 "IgnoreExtentions":["tmp"],
 "RemoveAfterSend":true
}
        EOH
        destination = "local/test.json"
      }

      #      resources {
      #        cpu = 5000
      #        memory = 128
      #        network {
      #          mbits = 10000
      #        }
      #      }

    } # task producer
  }
  #linux
}



