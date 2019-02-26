#ifndef ASAPO_WORKER_MOCKING_H
#define ASAPO_WORKER_MOCKING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/net_client.h"

namespace asapo {

class MockNetClient : public asapo::NetClient {
  public:

    Error GetData(const FileInfo* info, FileData* data) const noexcept override {
        return Error(GetData_t(info, data));
    }

    MOCK_CONST_METHOD2(GetData_t, ErrorInterface * (const FileInfo* info, FileData* data));

};

}



#endif //ASAPO_WORKER_MOCKING_H
