ASAPO_HOST_DIR=/var/tmp/asapo # you can change this if needed

docker exec asapo jobs-stop
docker stop asapo
rm -rf $ASAPO_HOST_DIR
