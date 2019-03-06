. ./clean_after_tests.sh
nomad stop asapo-filemon-producer
nomad run asapo-test_filemon_producer_toreceiver.nomad
sleep 1
nomad stop asapo-test
nomad run asapo-test_filegen_filemon_linuxonly.nomad

