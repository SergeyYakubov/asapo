cimport asapo_worker

cdef class PyDataBroker:
    cdef DataBroker* c_broker
    def get_data(self):
       return 0



cdef class PyDataBrokerFactory:
    cdef DataBrokerFactory c_factory
    def __cinit__(self):
        self.c_factory = DataBrokerFactory()
    def CreateServerBroker(self,server_name,source_path,beamtime_id,token):
        cdef Error err
        cdef unique_ptr[DataBroker] c_broker = self.c_factory.CreateServerBroker(server_name,source_path,beamtime_id,token,&err)
        broker = PyDataBroker()
        broker.c_broker =  c_broker.release()
        err_str = GetErrorString(&err)
        if err_str.strip():
            return None,err_str
        else:
            return broker,err_str

def CreateServerBroker(server_name,source_path,beamtime_id,token):
    factory = PyDataBrokerFactory()
    return factory.CreateServerBroker(server_name,source_path,beamtime_id,token)
