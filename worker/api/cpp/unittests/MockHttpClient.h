#ifndef HIDRA2_MOCKHTTPCLIENT_H
#define HIDRA2_MOCKHTTPCLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "http_client/http_client.h"
#include "worker/data_broker.h"


namespace hidra2 {

class MockHttpClient : public HttpClient {
  public:
    std::string Get(const std::string& uri, HttpCode* code, Error* err) const noexcept override {
        return Get_t(uri, code, err);
    }
    MOCK_CONST_METHOD3(Get_t,
                       std::string(const std::string& uri, HttpCode* code, Error* err));
};


}

#endif //HIDRA2_MOCKHTTPCLIENT_H
