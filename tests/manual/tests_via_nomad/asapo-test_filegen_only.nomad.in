job "asapo-filegen" {
  datacenters = [
    "dc1"]

  type = "batch"

  group "windows" {

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
          "1","1M","10","c:/tmp/asapo/test_in/test_folder/file_win"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/filegen_win.exe"
        mode = "file"
        destination = "local/filegen_win.exe"
      }
    }


  }
  #windows

  group "linux" {

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
          "1","1M","10","/tmp/asapo/test_in/test_folder/file_lin_"]
      }

      artifact {
        source = "http://nims.desy.de/extra/asapo/filegen_linux"
        mode = "file"
        destination = "local/filegen_linux"
      }
    }

  }
  #linux
}



