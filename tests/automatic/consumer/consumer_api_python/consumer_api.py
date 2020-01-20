from __future__ import print_function

import asapo_consumer
import json
import sys

def exit_on_noerr(name):
    print (name)
    sys.exit(1)


def assert_metaname(meta,compare,name):
    if meta['name'] != compare:
        print ("error at "+name)
        print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        sys.exit(1)

def assert_usermetadata(meta,name):
    if meta['meta']['test'] != 10:
        print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        print ("error at "+name)
        print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        sys.exit(1)


def assert_eq(val,expected,name):
    if val != expected:
        print ("error at "+name)
        print ('val: ', val,' expected: ',expected)
        sys.exit(1)


def check_broker_server_error(broker,group_id_new):
    try:
        broker.get_last(group_id_new, meta_only=True)
    except asapo_consumer.AsapoUnavailableServiceError as err:
        print(err)
        pass
    else:
        exit_on_noerr("AsapoBrokerServerError")

def check_single(broker,group_id_new):

    _, meta = broker.get_next(group_id_new, meta_only=True)
    assert_metaname(meta,"1","get next1")
    assert_usermetadata(meta,"get next1")

    data = broker.retrieve_data(meta)
    assert_eq(data.tostring().decode("utf-8"),"hello1","retrieve_data data")

    _, meta = broker.get_next(group_id_new, meta_only=True)
    assert_metaname(meta,"2","get next2")
    assert_usermetadata(meta,"get next2")

    _, meta = broker.get_last(group_id_new, meta_only=True)
    assert_metaname(meta,"5","get last1")
    assert_usermetadata(meta,"get last1")

    try:
        broker.get_by_id(30, group_id_new, meta_only=True)
    except asapo_consumer.AsapoEndOfStreamError:
        pass
    else:
        exit_on_noerr("get_by_id no data")


    try:
        _, meta = broker.get_next(group_id_new, meta_only=True)
    except asapo_consumer.AsapoEndOfStreamError:
        pass
    else:
        exit_on_noerr("get_next3")

    size = broker.get_current_size()
    assert_eq(size,5,"get_current_size")


    broker.reset_lastread_marker(group_id_new)

    _, meta = broker.get_next(group_id_new, meta_only=True)
    assert_metaname(meta,"1","get next4")
    assert_usermetadata(meta,"get next4")


    _, meta = broker.get_by_id(3, group_id_new, meta_only=True)
    assert_metaname(meta,"3","get get_by_id")
    assert_usermetadata(meta,"get get_by_id")

    _, meta = broker.get_next(group_id_new, meta_only=True)
    assert_metaname(meta,"2","get next5")
    assert_usermetadata(meta,"get next5")


    broker.set_lastread_marker(4, group_id_new)

    _, meta = broker.get_next(group_id_new, meta_only=True)
    assert_metaname(meta,"5","get next6")
    assert_usermetadata(meta,"get next6")

    try:
        broker.get_next("bla", meta_only=True)
    except asapo_consumer.AsapoWrongInputError as err:
        print(err)
        pass
    else:
        exit_on_noerr("wrong input")


    try:
        broker.get_last(group_id_new, meta_only=False)
    except asapo_consumer.AsapoLocalIOError as err:
        print(err)
        pass
    else:
        exit_on_noerr("io error")

    images = broker.query_images("meta.test = 10")
    assert_eq(len(images),5,"size of query answer 1")
    for image in images:
        assert_usermetadata(image,"query_images")


    images =  broker.query_images("meta.test = 10 AND name='1'")
    assert_eq(len(images),1,"size of query answer 2 ")

    for image in images:
        assert_usermetadata(image,"query_images")

    images = broker.query_images("meta.test = 11")
    assert_eq(len(images),0,"size of query answer 3 ")

    try:
        images = broker.query_images("bla")
    except:
        pass
    else:
        exit_on_noerr("wrong query")

    broker = asapo_consumer.create_server_broker("bla",path, beamtime,"",token,1000)
    try:
        broker.get_last(group_id_new, meta_only=True)
    except asapo_consumer.AsapoUnavailableServiceError as err:
        print(err)
        pass
    else:
        exit_on_noerr("AsapoBrokerServersNotFound")



def check_dataset(broker,group_id_new):
    id, metas = broker.get_next_dataset(group_id_new)
    assert_eq(id,1,"get_next_dataset1")
    assert_metaname(metas[0],"1_1","get nextdataset1 name1")
    assert_metaname(metas[1],"1_2","get nextdataset1 name2")
    assert_usermetadata(metas[0],"get nextdataset1 meta")

    data = broker.retrieve_data(metas[0])
    assert_eq(data.tostring().decode("utf-8"),"hello1","retrieve_data from dataset data")


    id, metas = broker.get_next_dataset(group_id_new)
    assert_eq(id,2,"get_next_dataset2")
    assert_metaname(metas[0],"2_1","get nextdataset2 name1")

    id, metas = broker.get_last_dataset(group_id_new)
    assert_eq(id,10,"get_last_dataset1")
    assert_metaname(metas[2],"10_3","get get_last_dataset1 name3")

    try:
        id, metas = broker.get_next_dataset(group_id_new)
    except asapo_consumer.AsapoEndOfStreamError as err:
        assert_eq(err.id_max,10,"get_next_dataset3 id_max")
        pass
    else:
        exit_on_noerr("get_next_dataset3 err")

    id, metas = broker.get_dataset_by_id(8,group_id_new)
    assert_eq(id,8,"get_dataset_by_id1 id")
    assert_metaname(metas[2],"8_3","get get_dataset_by_id1 name3")

    try:
        id, metas = broker.get_next_dataset(group_id_new)
    except:
        pass
    else:
        exit_on_noerr("get_next_dataset4 err")

source, path, beamtime, token, mode = sys.argv[1:]

broker = asapo_consumer.create_server_broker(source,path, beamtime,"",token,15000)

group_id_new = broker.generate_group_id()

if mode == "broker_server_error":
    check_broker_server_error(broker,group_id_new)


if mode == "single":
    check_single(broker,group_id_new)

if mode == "datasets":
    check_dataset(broker,group_id_new)

