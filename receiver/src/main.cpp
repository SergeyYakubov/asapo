#include "receiver.h"

int main (int argc, char* argv[]) {
  hidra2::Receiver* receiver = new hidra2::Receiver();

  receiver->start_listener("127.0.0.1", 8099);

  getchar();

  receiver->stop_listener();
  getchar();

  return 0;
}
