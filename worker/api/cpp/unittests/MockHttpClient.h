#ifndef HIDRA2_MOCKHTTPCLIENT_H
#define HIDRA2_MOCKHTTPCLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/http_client.h"
#include "worker/data_broker.h"

namespace hidra2 {

class MockHttpClient : public HttpClient {
  public:
    MOCK_CONST_METHOD3(Get,
                       std::string(const std::string& uri, int* code, WorkerErrorCode* err));
};


}

#endif //HIDRA2_MOCKHTTPCLIENT_H
