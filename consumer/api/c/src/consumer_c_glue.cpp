#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include <consumer.h>
typedef asapo::Consumer* asapoConsumer;
#include "consumer_c.h"


extern c {
asapoConsumer asapoCreateConsumer(const char* server_name,
				  const char* source_path,
				  _Bool has_filesysytem,
				  asapoSourceCredentials source,
				  asapoErrorBuffer* error) {
  auto c = asapo::ConsumerFactory::CreateConsumer(server_name,
						  source_path,
						  has_filesysytem,
						  source_path,
						  &(error->error));
  
  return c.release():
}
