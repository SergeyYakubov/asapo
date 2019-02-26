#ifndef ASAPO_NET_CLIENT_H
#define ASAPO_NET_CLIENT_H

#include "common/error.h"
#include "common/data_structs.h"

namespace asapo {

class NetClient {
  public:
    virtual Error GetData(const FileInfo* info, FileData* data) const noexcept = 0;
};

}

#endif //ASAPO_NET_CLIENT_H
