#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include "receiver.h"

const int Receiver::kMaxUnacceptedBacklog = 5;

Receiver::Receiver(sockaddr_in listen_address) {
  this->listen_address = listen_address;
}

void Receiver::start_listener() {
  sockfd = socket(AF_INET,SOCK_STREAM, 0);
  bind(sockfd, (struct sockaddr *)&listen_address, sizeof(listen_address));
  listen(sockfd, kMaxUnacceptedBacklog);
}

void Receiver::stop_listener() {
  close(sockfd);
}
