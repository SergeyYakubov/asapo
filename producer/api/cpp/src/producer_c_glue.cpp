#define __PRODUCER_C_INTERFACE_IMPLEMENTATION__

#include "asapo/common/internal/asapo_common_c_glue.h"
#include "asapo/asapo_producer.h"


//! handle for an asapo producer
/// created by asapo_create_producer()
/// free after use with asapo_free_handle()
/// all operations are done with asapo_producer_xxx() functions
/// \sa asapo::Producer
typedef AsapoHandlerHolder<asapo::Producer>* AsapoProducerHandle;

extern "C" {


}
