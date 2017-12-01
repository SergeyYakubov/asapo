#ifndef HIDRA2__PRODUCER_PRODUCER_H
#define HIDRA2__PRODUCER_PRODUCER_H

#include <string>
#include <memory>
#include <common/networking.h>
#include <system_wrappers/io.h>

namespace hidra2 {
enum ProducerError {
  PRODUCER_ERROR__OK,
  PRODUCER_ERROR__CONNECTION_NOT_READY,
  PRODUCER_ERROR__CHUNK_PROVIDER_NOT_READY_AT_START,
};

enum ProducerStatus {
  PRODUCER_STATUS__DISCONNECTED,
  PRODUCER_STATUS__CONNECTING,
  PRODUCER_STATUS__CONNECTED,
  PRODUCER_STATUS__SENDING,
  PRODUCER_STATUS__ERROR,
};

struct FileChunk {
  void *ptr;
  uint64_t start_byte;
  uint64_t chunk_size;
};

class Producer {
 public:
  static std::unique_ptr<Producer> create();

  //virtual ~Producer() = 0;

  virtual void __set_io(IO *io) = 0;

  virtual uint64_t get_version() const = 0;
  virtual ProducerStatus get_status() const = 0;

  virtual ProducerError connect_to_receiver(std::string receiver_address) = 0;
  virtual ProducerError send(std::string filename, uint64_t file_size, void *data) = 0;
};
}

#endif //HIDRA2__PRODUCER_PRODUCER_H
