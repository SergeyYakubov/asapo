#ifndef ASAPO_NET_CLIENT_H
#define ASAPO_NET_CLIENT_H

#include "asapo/common/error.h"
#include "asapo/common/data_structs.h"

namespace asapo {

class NetClient {
  public:
    virtual Error GetData(const MessageMeta* info, const std::string& request_sender_details, MessageData* data) = 0;
    virtual ~NetClient() = default;

};

}

#endif //ASAPO_NET_CLIENT_H
