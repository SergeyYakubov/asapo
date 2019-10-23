export asapo_host=`cat asapo_host`

dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream0
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream1
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream2
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream3
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream4
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream5
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream6
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream7
dockerrun -v `pwd`:/tmp/yakubov mongo mongo --host `curl -s $asapo_host:8400/discovery/mongo` --eval "db.dropDatabase()" asapo_test_stream8
