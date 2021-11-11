#ifndef ASAPO_MOCKHTTPCLIENT_H
#define ASAPO_MOCKHTTPCLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/http_client/http_client.h"

namespace asapo {

class MockHttpClient : public HttpClient {
  public:
    std::string Get(const std::string& uri, HttpCode* code, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto response = Get_t(uri, code, &error);
        err->reset(error);
        return response;
    }
    std::string Post(const std::string& uri, const std::string& cookie, const std::string& data, HttpCode* code,
                     Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto response = Post_t(uri, cookie, data, code, &error);
        err->reset(error);
        return response;
    }

    Error Post(const std::string& uri, const std::string& cookie, const std::string& input_data, MessageData* ouput_data,
               uint64_t output_data_size,
               HttpCode* response_code)  const noexcept override {
        return Error{PostReturnArray_t(uri, cookie, input_data, ouput_data, output_data_size, response_code)};
    };

    Error Post(const std::string&, const std::string&,
               const std::string&, std::string,
               HttpCode* )  const noexcept override {
        return nullptr;
    };


    std::string UrlEscape(const std::string& uri) const noexcept override {
        return UrlEscape_t(uri);
    }

    MOCK_METHOD(std::string, UrlEscape_t, (const std::string& uri), (const));


    MOCK_METHOD(std::string, Get_t, (const std::string& uri, HttpCode* code, ErrorInterface** err), (const));
    MOCK_METHOD(std::string, Post_t, (const std::string& uri, const std::string& cookie, const std::string& data, HttpCode* code, ErrorInterface** err), (const));
    MOCK_METHOD(ErrorInterface *, PostReturnArray_t, (const std::string& uri, const std::string& cookie, const std::string& input_data, MessageData* ouput_data, uint64_t output_data_size, HttpCode* code), (const));


};


}

#endif //ASAPO_MOCKHTTPCLIENT_H
