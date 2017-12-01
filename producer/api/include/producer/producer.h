#ifndef HIDRA2__PRODUCER_PRODUCER_H
#define HIDRA2__PRODUCER_PRODUCER_H

#include <string>

namespace HIDRA2 {
class Producer {
  private:
    static unsigned long kinit_count_;
    Producer();
  public:
    static const uint32_t VERSION;

    Producer(const Producer&) = delete;
    Producer& operator=(const Producer&) = delete;

    static Producer* CreateProducer(std::string receiver_address);
};
}

#endif //HIDRA2__PRODUCER_PRODUCER_H
