nomad run receiver_tcp.nmd
nomad run authorizer.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run nginx.nmd
nomad run file_transfer.nmd

while true
do
  sleep 1
  echo starting services
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-receiver?protocol=v0.1 --stderr - | grep 127.0.0.1  || continue
  echo nginx,discovery, recevier started
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.1 --stderr - | grep 127.0.0.1 || continue
  echo broker started
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-file-transfer?protocol=v0.1 --stderr -  | grep 127.0.0.1 || continue
  echo file transfer started
  break
done

echo services ready