#include <system_wrappers/system_io.h>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;
const hidra2::IO *hidra2::ProducerImpl::kDefaultIO = new hidra2::SystemIO();

hidra2::FileReferenceId hidra2::ProducerImpl::kGlobalReferenceId = 0;

hidra2::ProducerImpl::ProducerImpl() {
  __set_io(ProducerImpl::kDefaultIO);
}

void hidra2::ProducerImpl::__set_io(hidra2::IO *io) {
  this->io = io;
}

uint64_t hidra2::ProducerImpl::get_version() const {
  return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
  return PRODUCER_STATUS__CONNECTED;
}

hidra2::ProducerError hidra2::ProducerImpl::connect_to_receiver(std::string receiver_address) {
  return PRODUCER_ERROR__OK;
}

hidra2::ProducerError hidra2::ProducerImpl::send(std::string filename, uint64_t file_size, void *data) {
}
