import asapo_consumer
import h5py
import numpy as np
import matplotlib.pyplot as plt


#d1 = np.random.random(size = (20))
#f = h5py.File("mytestfile1.hdf5", "w")
#dset = f.create_dataset("mydataset", data = d1)
#f.close()

broker, err = asapo_consumer.create_server_broker("psana002:8400", "/tmp", True, "asapo_test2","","yzgAcLmijSLWIm8dBiGNCbc0i42u5HSm-zR6FRqo__Y=", 1000000)

last_id = 0
while True:
    data, meta, err = broker.get_last(meta_only=False)
    id = meta['_id']
    if id == last_id:
        continue
    fid = h5py.h5f.open_file_image(data)
    f = h5py.File(fid)
    data1 = np.array(f['mydataset'])
    print(data1)
    plt.plot(data1)
    plt.ylabel('some numbers')
    plt.show()
    last_id = id
    f.close()


#alternative - but tobytes creates an additional copy - not nice.
#import tables
#h5file1 = tables.open_file("in-memory-sample.h5", driver="H5FD_CORE",
#                              driver_core_image=data.tobytes(),
#                              driver_core_backing_store=0)
#data2 = h5file1.root.mydataset.read()



