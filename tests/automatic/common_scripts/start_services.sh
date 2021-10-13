nomad run discovery.nmd
nomad run nginx.nmd

while true
do
  sleep 1
  echo starting nginx and discovery
  curl --silent 127.0.0.1:8400/asapo-discovery/asapo-mongodb --stderr - | grep 127.0.0.1  || continue
  echo nginx and discovery started
  break
done

nomad run authorizer.nmd
nomad run file_transfer.nmd
nomad run receiver_tcp.nmd
nomad run broker.nmd

while true
do
  sleep 1
  echo starting services
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-receiver?protocol=v0.1 --stderr - | grep 127.0.0.1  || continue
  echo recevier started
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.1 --stderr - | grep 127.0.0.1 || continue
  echo broker started
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-file-transfer?protocol=v0.1 --stderr -  | grep 127.0.0.1 || continue
  echo file transfer started
  break
done

echo services ready