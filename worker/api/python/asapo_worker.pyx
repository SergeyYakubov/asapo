#distutils: language=c++

cimport asapo_worker
import numpy as np
cimport numpy as np
import json
from cpython.version cimport PY_MAJOR_VERSION

np.import_array()

cdef str _str(b):
    if PY_MAJOR_VERSION < 3:
        return b
    return b.decode("utf-8")

cdef bytes _bytes(s):
    if type(s) is bytes:
        return s

    elif isinstance(s, unicode):
        return (<unicode>s).encode('utf-8')

    else:
        raise TypeError("Could not convert to unicode.")


cdef class PyDataBroker:
    cdef DataBroker* c_broker
    def _op(self, op, meta_only):
        cdef FileInfo info
        cdef FileData data
        cdef Error err
        cdef np.npy_intp dims[1]
        if op == "next":
            err =  self.c_broker.GetNext(&info,<FileData*>NULL if meta_only else &data)
        elif op == "last":
            err =  self.c_broker.GetLast(&info,<FileData*>NULL if meta_only else &data)
        err_str = _str(GetErrorString(&err))
        if err_str.strip():
            return None,None,err_str
        info_str = _str(info.Json())
        meta = json.loads(info_str)
        if meta_only:
            return None,meta,None
        cdef char* ptr = <char*> data.release()
        dims[0] = meta['size']
        del meta['buf_id']
        del meta['source']
        del meta['lastchange']
        arr =  np.PyArray_SimpleNewFromData(1, dims, np.NPY_BYTE, ptr)
        return arr,meta,None


    def get_next(self, meta_only = True):
        return self._op("next",meta_only)
    def get_last(self, meta_only = True):
        return self._op("last",meta_only)


cdef class PyDataBrokerFactory:
    cdef DataBrokerFactory c_factory
    def __cinit__(self):
        self.c_factory = DataBrokerFactory()
    def create_server_broker(self,server_name,source_path,beamtime_id,token,timeout):
        cdef Error err
        cdef unique_ptr[DataBroker] c_broker = self.c_factory.CreateServerBroker(server_name,source_path,beamtime_id,token,&err)
        broker = PyDataBroker()
        broker.c_broker =  c_broker.release()
        broker.c_broker.SetTimeout(timeout)
        err_str = GetErrorString(&err)
        if err_str.strip():
            return None,err_str
        else:
            return broker,err_str

def create_server_broker(server_name,source_path,beamtime_id,token,timeout):
    factory = PyDataBrokerFactory()
    return factory.create_server_broker(_bytes(server_name),_bytes(source_path),_bytes(beamtime_id),_bytes(token),timeout)
