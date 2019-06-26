from __future__ import print_function

import asapo_worker
import json
import sys

def assert_noterr(err, name):
    if err != None:
        print (name + ' err: ', err)
        sys.exit(1)

def assert_err(err, name):
    if err == None:
        print (name + ' err: ', err)
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


source, path, beamtime, token = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(source,path, beamtime,token,1000)

group_id_new, err = broker.generate_group_id()
assert_noterr(err, "generate_group")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_noterr(err, "get_next")
assert_metaname(meta,"1","get next1")
assert_usermetadata(meta,"get next1")


_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_noterr(err, "get_next2")
assert_metaname(meta,"2","get next2")
assert_usermetadata(meta,"get next2")

_, meta, err = broker.get_last(group_id_new, meta_only=True)
assert_noterr(err, "get_last1")
assert_metaname(meta,"5","get last1")
assert_usermetadata(meta,"get last1")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_err(err, "get_next3")

size,err = broker.get_ndatasets()
assert_noterr(err, "get_ndatasets")
assert_eq(size,5,"get_ndatasets")


err = broker.reset_counter(group_id_new)
assert_noterr(err, "reset_counter")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_noterr(err, "get_next4")
assert_metaname(meta,"1","get next4")
assert_usermetadata(meta,"get next4")


_, meta, err = broker.get_by_id(3, group_id_new, meta_only=True)
assert_noterr(err, "get_by_id")
assert_metaname(meta,"3","get get_by_id")
assert_usermetadata(meta,"get get_by_id")

_, meta, err = broker.get_next(group_id_new, meta_only=True)
assert_noterr(err, "get_next5")
assert_metaname(meta,"4","get next5")
assert_usermetadata(meta,"get next5")


images,err = broker.query_images("meta.test = 10")
assert_noterr(err, "query1")
assert_eq(len(images),5,"size of query answer 1")
for image in images:
    assert_usermetadata(image,"query_images")


images,err = broker.query_images("meta.test = 10 AND name='1'")
assert_eq(len(images),1,"size of query answer 2 ")
assert_noterr(err, "query2")

for image in images:
    assert_usermetadata(image,"query_images")

images,err = broker.query_images("meta.test = 11")
assert_eq(len(images),0,"size of query answer 3 ")
assert_noterr(err, "query3")

images,err = broker.query_images("bla")
assert_err(err, "wrong query")

