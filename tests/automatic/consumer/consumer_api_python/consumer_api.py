from __future__ import print_function

import asapo_consumer
import json
import sys


def exit_on_noerr(name):
    print(name)
    sys.exit(1)


def assert_metaname(meta, compare, name):
    print("asserting meta for " + name)
    if meta['name'] != compare:
        print("error at " + name)
        print('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        sys.exit(1)


def assert_usermetadata(meta, name):
    print("asserting usermetadata for " + name)
    if meta['meta']['test'] != 10:
        print('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        print("error at " + name)
        print('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        sys.exit(1)


def assert_eq(val, expected, name):
    print("asserting eq for " + name)
    if val != expected:
        print("error at " + name)
        print('val: ', val, ' expected: ', expected)
        sys.exit(1)


def check_file_transfer_service(broker, group_id):
    broker.set_timeout(1000)
    data, meta = broker.get_by_id(1, group_id, meta_only=False)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "check_file_transfer_service ok")
    data, meta = broker.get_by_id(1, group_id, "streamfts", meta_only=False)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "check_file_transfer_service with auto size ok")


def check_single(broker, group_id):
    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next1")
    assert_usermetadata(meta, "get next1")

    broker.set_timeout(1000)

    data = broker.retrieve_data(meta)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "retrieve_data data")

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next2")
    assert_usermetadata(meta, "get next2")

    _, meta = broker.get_last(group_id, meta_only=True)
    assert_metaname(meta, "5", "get last1")
    assert_usermetadata(meta, "get last1")

    try:
        broker.get_by_id(30, group_id, meta_only=True)
    except asapo_consumer.AsapoEndOfStreamError:
        pass
    else:
        exit_on_noerr("get_by_id no data")

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "3", "get next3")


    size = broker.get_current_size()
    assert_eq(size, 5, "get_current_size")

    broker.reset_lastread_marker(group_id)

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next4")
    assert_usermetadata(meta, "get next4")

    _, meta = broker.get_by_id(3, group_id, meta_only=True)
    assert_metaname(meta, "3", "get get_by_id")
    assert_usermetadata(meta, "get get_by_id")

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next5")
    assert_usermetadata(meta, "get next5")

    broker.set_lastread_marker(4, group_id)

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "5", "get next6")
    assert_usermetadata(meta, "get next6")

    try:
        broker.get_next("_wrong_group_name", meta_only=True)
    except asapo_consumer.AsapoWrongInputError as err:
        print(err)
        pass
    else:
        exit_on_noerr("should give wrong input error")

    try:
        broker.get_last(group_id, meta_only=False)
    except asapo_consumer.AsapoLocalIOError as err:
        print(err)
        pass
    else:
        exit_on_noerr("io error")

    _, meta = broker.get_next(group_id, "stream1", meta_only=True)
    assert_metaname(meta, "11", "get next stream1")

    _, meta = broker.get_next(group_id, "stream2", meta_only=True)
    assert_metaname(meta, "21", "get next stream2")

    substreams = broker.get_substream_list("")
    assert_eq(len(substreams), 4, "number of substreams")
    print(substreams)
    assert_eq(substreams[0]["name"], "default", "substreams_name1")
    assert_eq(substreams[1]["name"], "streamfts", "substreams_name2")
    assert_eq(substreams[2]["name"], "stream1", "substreams_name2")
    assert_eq(substreams[3]["name"], "stream2", "substreams_name3")
    assert_eq(substreams[1]["timestampCreated"], 1000, "substreams_timestamp2")

    # acks
    try:
        id = broker.get_last_acknowledged_tuple_id(group_id)
    except asapo_consumer.AsapoNoDataError as err:
        print(err)
        pass
    else:
        exit_on_noerr("get_last_acknowledged_tuple_id")

    nacks = broker.get_unacknowledged_tuple_ids(group_id)
    assert_eq(len(nacks), 5, "nacks default stream size = 5")

    broker.acknowledge(group_id, 1)

    nacks = broker.get_unacknowledged_tuple_ids(group_id)
    assert_eq(len(nacks), 4, "nacks default stream size = 4")

    id = broker.get_last_acknowledged_tuple_id(group_id)
    assert_eq(id, 1, "last ack default stream id = 1")

    broker.acknowledge(group_id, 1, "stream1")
    nacks = broker.get_unacknowledged_tuple_ids(group_id)
    assert_eq(len(nacks), 4, "nacks stream1 size = 4 after ack")

    # neg acks
    broker.reset_lastread_marker(group_id)
    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next neg ack before resend")
    broker.reset_lastread_marker(group_id)
    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next neg ack with resend")

    # resend
    broker.reset_lastread_marker(group_id)
    broker.set_resend_nacs(True, 0, 1)
    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next before resend")

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next with resend")

    _, meta = broker.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next after resend")

    # images

    images = broker.query_images("meta.test = 10")
    assert_eq(len(images), 5, "size of query answer 1")
    for image in images:
        assert_usermetadata(image, "query_images")

    images = broker.query_images("meta.test = 10 AND name='1'")
    assert_eq(len(images), 1, "size of query answer 2 ")

    for image in images:
        assert_usermetadata(image, "query_images")

    images = broker.query_images("meta.test = 11")
    assert_eq(len(images), 0, "size of query answer 3 ")

    try:
        images = broker.query_images("bla")
    except:
        pass
    else:
        exit_on_noerr("wrong query")

    broker = asapo_consumer.create_server_broker("bla", path, True, beamtime, "", token, 1000)
    try:
        broker.get_last(group_id, meta_only=True)
    except asapo_consumer.AsapoUnavailableServiceError as err:
        print(err)
        pass
    else:
        exit_on_noerr("AsapoBrokerServersNotFound")


def check_dataset(broker, group_id):
    res = broker.get_next_dataset(group_id)
    assert_eq(res['id'], 1, "get_next_dataset1")
    assert_metaname(res['content'][0], "1_1", "get nextdataset1 name1")
    assert_metaname(res['content'][1], "1_2", "get nextdataset1 name2")
    assert_usermetadata(res['content'][0], "get nextdataset1 meta")

    broker.set_timeout(1000)

    data = broker.retrieve_data(res['content'][0])
    assert_eq(data.tostring().decode("utf-8"), "hello1", "retrieve_data from dataset data")

    res = broker.get_next_dataset(group_id)
    assert_eq(res['id'], 2, "get_next_dataset2")
    assert_metaname(res['content'][0], "2_1", "get nextdataset2 name1")

    res = broker.get_last_dataset(group_id)
    assert_eq(res['id'], 10, "get_last_dataset1")
    assert_eq(res['expected_size'], 3, "get_last_dataset1 size ")
    assert_metaname(res['content'][2], "10_3", "get get_last_dataset1 name3")

    res = broker.get_next_dataset(group_id)
    assert_eq(res['id'], 3, "get_next_dataset3")

    res = broker.get_dataset_by_id(8, group_id)
    assert_eq(res['id'], 8, "get_dataset_by_id1 id")
    assert_metaname(res['content'][2], "8_3", "get get_dataset_by_id1 name3")

    # incomplete datesets without min_size given
    try:
        broker.get_next_dataset(group_id, "incomplete")
    except asapo_consumer.AsapoPartialDataError as err:
        assert_eq(err.partial_data['expected_size'], 3, "get_next_dataset incomplete expected size")
        assert_eq(err.partial_data['id'], 1, "get_next_dataset incomplete id")
        assert_eq(err.partial_data['content'][0]['name'], '1_1', "get_next_dataset content 1")
        assert_eq(err.partial_data['content'][1]['name'], '1_2', "get_next_dataset content 2")
        pass
    else:
        exit_on_noerr("get_next_dataset incomplete err")

    try:
        broker.get_dataset_by_id(2, group_id, "incomplete")
    except asapo_consumer.AsapoPartialDataError as err:
        assert_eq(err.partial_data['expected_size'], 3, "get_next_dataset incomplete expected size")
        assert_eq(err.partial_data['id'], 2, "get_next_dataset incomplete id")
        assert_eq(err.partial_data['content'][0]['name'], '2_1', "get_next_dataset content 1")
        assert_eq(err.partial_data['content'][1]['name'], '2_2', "get_next_dataset content 2")
        pass
    else:
        exit_on_noerr("get_next_dataset incomplete err")

    try:
        broker.get_last_dataset(group_id, "incomplete")
    except asapo_consumer.AsapoEndOfStreamError as err:
        pass
    else:
        exit_on_noerr("get_last_dataset incomplete err")
    # incomplete with min_size given
    res = broker.get_next_dataset(group_id, "incomplete", min_size=2)
    assert_eq(res['id'], 2, "get_next_dataset incomplete with minsize")

    res = broker.get_last_dataset(group_id, "incomplete", min_size=2)
    assert_eq(res['id'], 5, "get_last_dataset incomplete with minsize")

    res = broker.get_dataset_by_id(2, group_id, "incomplete", min_size=1)
    assert_eq(res['id'], 2, "get_dataset_by_id incomplete with minsize")


source, path, beamtime, token, mode = sys.argv[1:]

broker = asapo_consumer.create_server_broker(source, path, True, beamtime, "", token, 60000)
broker_fts = asapo_consumer.create_server_broker(source, path, False, beamtime, "", token, 60000)

group_id = broker.generate_group_id()

group_id_fts = broker_fts.generate_group_id()

if mode == "single":
    check_single(broker, group_id)
    check_file_transfer_service(broker_fts, group_id_fts)

if mode == "datasets":
    check_dataset(broker, group_id)

print("tests done")
sys.exit(0)
