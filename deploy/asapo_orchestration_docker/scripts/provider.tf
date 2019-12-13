provider "nomad" {
  address = "http://localhost:4646"
#  secret_id = "${chomp(file("/var/nomad/token"))}"
}

