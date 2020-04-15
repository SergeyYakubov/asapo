export PYTHONPATH=/Users/yakubov/projects/asapo/cmake-build-debug/consumer/api/python
export token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=
python3 consumer_api.py gest-k8s-test2.desy.de/yakser /test_offline/test_facility/gpfs/test/2019/data/asapo_test asapo_test $token
#python3 getnext.py gest-k8s-test2.desy.de/yakser /test_offline/test_facility/gpfs/test/2019/data/asapo_test asapo_test $token new