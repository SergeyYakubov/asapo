from __future__ import print_function

import asapo_consumer
import json
import sys
import time
from threading import Thread

thread_res = 0

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


def check_file_transfer_service(consumer, group_id):
    consumer.set_timeout(1000)
    data, meta = consumer.get_by_id(1, meta_only=False)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "check_file_transfer_service ok")
    data, meta = consumer.get_by_id(1, "streamfts", meta_only=False)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "check_file_transfer_service with auto size ok")


def check_single(consumer, group_id):
    global thread_res
    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next1")
    assert_usermetadata(meta, "get next1")

    consumer.set_timeout(1000)

    data = consumer.retrieve_data(meta)
    assert_eq(data.tostring().decode("utf-8"), "hello1", "retrieve_data data")

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next2")
    assert_usermetadata(meta, "get next2")

    _, meta = consumer.get_last(meta_only=True)
    assert_metaname(meta, "5", "get last1")
    assert_usermetadata(meta, "get last1")

    try:
        consumer.get_by_id(30, meta_only=True)
    except asapo_consumer.AsapoEndOfStreamError:
        pass
    else:
        exit_on_noerr("get_by_id no data")

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "3", "get next3")


    size = consumer.get_current_size()
    assert_eq(size, 5, "get_current_size")

    consumer.reset_lastread_marker(group_id)

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next4")
    assert_usermetadata(meta, "get next4")

    _, meta = consumer.get_by_id(3, meta_only=True)
    assert_metaname(meta, "3", "get get_by_id")
    assert_usermetadata(meta, "get get_by_id")

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next5")
    assert_usermetadata(meta, "get next5")

    consumer.set_lastread_marker(group_id,4)

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "5", "get next6")
    assert_usermetadata(meta, "get next6")

    try:
        consumer.get_next("_wrong_group_name", meta_only=True)
    except asapo_consumer.AsapoWrongInputError as err:
        print(err)
        pass
    else:
        exit_on_noerr("should give wrong input error")

    try:
        consumer.get_last(meta_only=False)
    except asapo_consumer.AsapoLocalIOError as err:
        print(err)
        pass
    else:
        exit_on_noerr("io error")

    _, meta = consumer.get_next(group_id, "stream1", meta_only=True)
    assert_metaname(meta, "11", "get next stream1")

    _, meta = consumer.get_next(group_id, "stream2", meta_only=True)
    assert_metaname(meta, "21", "get next stream2")

    streams = consumer.get_stream_list("")
    assert_eq(len(streams), 4, "number of streams")
    print(streams)
    assert_eq(streams[0]["name"], "default", "streams_name1")
    assert_eq(streams[1]["name"], "streamfts", "streams_name2")
    assert_eq(streams[2]["name"], "stream1", "streams_name2")
    assert_eq(streams[3]["name"], "stream2", "streams_name3")
    assert_eq(streams[1]["timestampCreated"], 1000, "streams_timestamp2")

    # acks
    try:
        id = consumer.get_last_acknowledged_message(group_id)
    except asapo_consumer.AsapoNoDataError as err:
        print(err)
        pass
    else:
        exit_on_noerr("get_last_acknowledged_message")

    nacks = consumer.get_unacknowledged_messages(group_id)
    assert_eq(len(nacks), 5, "nacks default stream size = 5")

    consumer.acknowledge(group_id, 1)

    nacks = consumer.get_unacknowledged_messages(group_id)
    assert_eq(len(nacks), 4, "nacks default stream size = 4")

    id = consumer.get_last_acknowledged_message(group_id)
    assert_eq(id, 1, "last ack default stream id = 1")

    consumer.acknowledge(group_id, 1, "stream1")
    nacks = consumer.get_unacknowledged_messages(group_id)
    assert_eq(len(nacks), 4, "nacks stream1 size = 4 after ack")

    # neg acks
    consumer.reset_lastread_marker(group_id)
    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next neg ack before resend")
    consumer.reset_lastread_marker(group_id)
    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next neg ack with resend")

    # resend
    consumer.reset_lastread_marker(group_id)
    consumer.set_resend_nacs(True, 0, 1)
    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next before resend")

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "1", "get next with resend")

    _, meta = consumer.get_next(group_id, meta_only=True)
    assert_metaname(meta, "2", "get next after resend")

    # messages

    messages = consumer.query_messages("meta.test = 10")
    assert_eq(len(messages), 5, "size of query answer 1")
    for message in messages:
        assert_usermetadata(message, "query_messages")

    messages = consumer.query_messages("meta.test = 10 AND name='1'")
    assert_eq(len(messages), 1, "size of query answer 2 ")

    for message in messages:
        assert_usermetadata(message, "query_messages")

    messages = consumer.query_messages("meta.test = 11")
    assert_eq(len(messages), 0, "size of query answer 3 ")

    try:
        messages = consumer.query_messages("bla")
    except:
        pass
    else:
        exit_on_noerr("wrong query")

    consumer = asapo_consumer.create_consumer("bla", path, True, beamtime, "", token, 1000)
    try:
        consumer.get_last(meta_only=True)
    except asapo_consumer.AsapoUnavailableServiceError as err:
        print(err)
        pass
    else:
        exit_on_noerr("AsapoconsumerServersNotFound")

    try:
        asapo_consumer.create_consumer("", "", True, "", "", "", 1000)
    except asapo_consumer.AsapoWrongInputError as err:
        print(err)
        pass
    else:
        exit_on_noerr("should be AsapoWrongInputError")

# interrupt
    thread_res = 0
    def long_call(consumer):
        global thread_res
        try:
            consumer.get_last(meta_only=True)
            thread_res = 1
        except asapo_consumer.AsapoInterruptedTransactionError as err:
            global res
            print(err)
            thread_res = 2
            pass
        else:
            print("interrupt test failed")
            thread_res = 3
            pass

    consumer = asapo_consumer.create_consumer("bla", path, True, beamtime, "", token, 60000)
    t = Thread(target =  long_call, args =  (consumer,) )
    t.start()
    time.sleep(1)
    consumer.interrupt_current_operation()
    t.join()
    assert_eq(thread_res, 2, "long call res")



def check_dataset(consumer, group_id):
    res = consumer.get_next_dataset(group_id)
    assert_eq(res['id'], 1, "get_next_dataset1")
    assert_metaname(res['content'][0], "1_1", "get nextdataset1 name1")
    assert_metaname(res['content'][1], "1_2", "get nextdataset1 name2")
    assert_usermetadata(res['content'][0], "get nextdataset1 meta")

    consumer.set_timeout(1000)

    data = consumer.retrieve_data(res['content'][0])
    assert_eq(data.tostring().decode("utf-8"), "hello1", "retrieve_data from dataset data")

    res = consumer.get_next_dataset(group_id)
    assert_eq(res['id'], 2, "get_next_dataset2")
    assert_metaname(res['content'][0], "2_1", "get nextdataset2 name1")

    res = consumer.get_last_dataset()
    assert_eq(res['id'], 10, "get_last_dataset1")
    assert_eq(res['expected_size'], 3, "get_last_dataset1 size ")
    assert_metaname(res['content'][2], "10_3", "get get_last_dataset1 name3")

    res = consumer.get_next_dataset(group_id)
    assert_eq(res['id'], 3, "get_next_dataset3")

    res = consumer.get_dataset_by_id(8)
    assert_eq(res['id'], 8, "get_dataset_by_id1 id")
    assert_metaname(res['content'][2], "8_3", "get get_dataset_by_id1 name3")

    # incomplete datesets without min_size given
    try:
        consumer.get_next_dataset(group_id, "incomplete")
    except asapo_consumer.AsapoPartialDataError as err:
        assert_eq(err.partial_data['expected_size'], 3, "get_next_dataset incomplete expected size")
        assert_eq(err.partial_data['id'], 1, "get_next_dataset incomplete id")
        assert_eq(err.partial_data['content'][0]['name'], '1_1', "get_next_dataset content 1")
        assert_eq(err.partial_data['content'][1]['name'], '1_2', "get_next_dataset content 2")
        pass
    else:
        exit_on_noerr("get_next_dataset incomplete err")

    try:
        consumer.get_dataset_by_id(2, "incomplete")
    except asapo_consumer.AsapoPartialDataError as err:
        assert_eq(err.partial_data['expected_size'], 3, "get_next_dataset incomplete expected size")
        assert_eq(err.partial_data['id'], 2, "get_next_dataset incomplete id")
        assert_eq(err.partial_data['content'][0]['name'], '2_1', "get_next_dataset content 1")
        assert_eq(err.partial_data['content'][1]['name'], '2_2', "get_next_dataset content 2")
        pass
    else:
        exit_on_noerr("get_next_dataset incomplete err")

    try:
        consumer.get_last_dataset("incomplete")
    except asapo_consumer.AsapoEndOfStreamError as err:
        pass
    else:
        exit_on_noerr("get_last_dataset incomplete err")
    # incomplete with min_size given
    res = consumer.get_next_dataset(group_id, "incomplete", min_size=2)
    assert_eq(res['id'], 2, "get_next_dataset incomplete with minsize")

    res = consumer.get_last_dataset("incomplete", min_size=2)
    assert_eq(res['id'], 5, "get_last_dataset incomplete with minsize")

    res = consumer.get_dataset_by_id(2, "incomplete", min_size=1)
    assert_eq(res['id'], 2, "get_dataset_by_id incomplete with minsize")


source, path, beamtime, token, mode = sys.argv[1:]

consumer = asapo_consumer.create_consumer(source, path, True, beamtime, "", token, 60000)
consumer_fts = asapo_consumer.create_consumer(source, path, False, beamtime, "", token, 60000)

group_id = consumer.generate_group_id()

group_id_fts = consumer_fts.generate_group_id()

if mode == "single":
    check_single(consumer, group_id)
    check_file_transfer_service(consumer_fts, group_id_fts)

if mode == "datasets":
    check_dataset(consumer, group_id)

print("tests done")
sys.exit(0)
