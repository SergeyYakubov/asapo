#include <system_wrappers/system_io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;


hidra2::FileReferenceId hidra2::ProducerImpl::kGlobalReferenceId = 0;

hidra2::ProducerImpl::ProducerImpl() {
  __set_io(ProducerImpl::kDefaultIO);
}

uint64_t hidra2::ProducerImpl::get_version() const {
  return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
  return PRODUCER_STATUS__CONNECTED;
}

hidra2::ProducerError hidra2::ProducerImpl::connect_to_receiver(std::string receiver_address) {
  int client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  sockaddr_in socket_address {};
  socket_address.sin_addr.s_addr = inet_addr(receiver_address.c_str());
  socket_address.sin_port = htons(8099);
  socket_address.sin_family = AF_INET;

  connect(client_fd, (struct sockaddr *)&socket_address, sizeof(socket_address));

  void* a = malloc(1024*1024);

  while(true) {
    recv(client_fd, a, 1024*1024, 0);
  }

  return PRODUCER_ERROR__OK;
}

hidra2::ProducerError hidra2::ProducerImpl::send(std::string filename, uint64_t file_size, void *data) {
}
