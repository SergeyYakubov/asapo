docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock -v `pwd`/jobs:/usr/local/nomad_jobs --name asapo --net=host -v /tmp/nomad:/tmp/nomad -v /var/lib/docker:/var/lib/docker -d yakser/asapo-cluster

