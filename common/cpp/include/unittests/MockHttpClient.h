#ifndef ASAPO_MOCKHTTPCLIENT_H
#define ASAPO_MOCKHTTPCLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "http_client/http_client.h"

namespace asapo {

class MockHttpClient : public HttpClient {
  public:
    std::string Get(const std::string& uri, HttpCode* code, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto response = Get_t(uri, code, &error);
        err->reset(error);
        return response;
    }
    std::string Post(const std::string& uri, const std::string& data, HttpCode* code, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto response = Post_t(uri, data, code, &error);
        err->reset(error);
        return response;
    }
    MOCK_CONST_METHOD3(Get_t,
                       std::string(const std::string& uri, HttpCode* code, ErrorInterface** err));
    MOCK_CONST_METHOD4(Post_t,
                       std::string(const std::string& uri, const std::string& data, HttpCode* code, ErrorInterface** err));

};


}

#endif //ASAPO_MOCKHTTPCLIENT_H
