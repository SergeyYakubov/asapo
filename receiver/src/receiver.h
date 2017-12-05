#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#import <string>


class Receiver {
 private:
  static const int kMaxUnacceptedBacklog;
public:
  int sockfd, newsockfd, n, pid;
  Receiver(sockaddr_in listen_address);

  struct sockaddr listen_address;
  void start_listener();
  void stop_listener();
};


#endif //HIDRA2_RECEIVER_H
