. ./clean_after_tests.sh
nomad stop asapo-test
nomad run  asapo-test_dummy_producer_linux_noworker.nomad

