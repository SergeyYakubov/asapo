#ifndef HIDRA2__PRODUCER_PRODUCER_H
#define HIDRA2__PRODUCER_PRODUCER_H

#include <string>

namespace HIDRA2
{
    enum ProducerError {
        PRODUCER_ERROR__OK,
    };

    class Producer
    {
    public:
        static Producer* create();

        virtual uint64_t get_version() = 0;
        //virtual ProducerError connect(std::string receiver_address) = 0;
        //virtual ProducerError sendfile(std::string receiver_address) = 0;
    };
}

#endif //HIDRA2__PRODUCER_PRODUCER_H
