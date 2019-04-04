from __future__ import print_function

import asapo_worker
import json
import sys

def assert_err(err,name):
    if err != None:
        print (name + ' err: ', err)
        sys.exit(1)

def assert_noterr(err,name):
    if err == None:
        print (name + ' err: ', err)
        sys.exit(1)


def assert_metaname(meta,compare,name):
    if meta['name'] != compare:
        print ("error at "+name)
        print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
        sys.exit(1)

def assert_eq(val,expected,name):
    if val != expected:
        print ("error at "+name)
        print ('val: ', val,' expected: ',expected)
        sys.exit(1)


source, path, beamtime, token = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(source,path, beamtime,token,1000)

group_id_new, err = broker.generate_group_id()
assert_err(err,"generate_group")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_err(err,"get_next")
assert_metaname(meta,"1","get next1")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_err(err,"get_next2")
assert_metaname(meta,"2","get next2")

_, meta, err = broker.get_last(group_id_new, meta_only=True)
assert_err(err,"get_last1")
assert_metaname(meta,"5","get last1")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_noterr(err,"get_next3")

size,err = broker.get_ndatasets()
assert_err(err,"get_ndatasets")
assert_eq(size,5,"get_ndatasets")


err = broker.reset_counter(group_id_new)
assert_err(err,"reset_counter")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_err(err,"get_next4")
assert_metaname(meta,"1","get next4")


_, meta, err = broker.get_by_id(3, group_id_new, meta_only=True)
assert_err(err,"get_by_id")
assert_metaname(meta,"3","get get_by_id")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_err(err,"get_next5")
assert_metaname(meta,"4","get next5")
