#ifndef ASAPO_TCP_CLIENT_H
#define ASAPO_TCP_CLIENT_H

#include "net_client.h"

namespace asapo {

class TcpClient : public NetClient {
  public:
    Error GetData(const FileInfo* info, FileData* data) const noexcept override;

};

}

#endif //ASAPO_TCP_CLIENT_H
