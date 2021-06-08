#include <iostream>
#include <vector>
#include "asapo/consumer/consumer.h"
#include "testing.h"
#include "../../../consumer/api/cpp/src/consumer_impl.h"
#include "asapo/preprocessor/definitions.h"
#include "asapo/io/io_factory.h"
#include "asapo/io/io.h"

struct Args {
    std::string uri_authorizer;
    std::string uri_fts;
    std::string folder;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string uri_authorizer{argv[1]};
    std::string uri_fts{argv[2]};
    std::string folder{argv[3]};
    return Args{uri_authorizer, uri_fts, folder};
}


int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);
    auto token =
        "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTkycDFiaXB0MzBub3AwcTNlZyIsInN1YiI6ImJ0X2FhYSIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbInJlYWQiXX19.dt3ifrG3zqQP4uM2kaoe7ydDjUdFeasOB07fVRfFApE"; //token for aaa
    std::string authorize_request = "{\"Folder\":\"" + args.folder + "\",\"BeamtimeId\":\"aaa\",\"Token\":\"" + token +
                                    "\"}";
    asapo::Error err;
    auto consumer = asapo::ConsumerFactory::CreateConsumer(args.uri_authorizer,
                    "",
                    true,
                    asapo::SourceCredentials{asapo::SourceType::kProcessed, "", "",
                                             "", ""},
                    &err);
    auto consumer_impl = static_cast<asapo::ConsumerImpl*>(consumer.get());
    M_AssertEq(nullptr, err);

    asapo::HttpCode code;
    std::string response;
    std::string input_data;
    auto folder_token = consumer_impl->httpclient__->Post(args.uri_authorizer + "/v0.1/folder", "", authorize_request,
                        &code,
                        &err);
    if (err) {
        std::cout << err->Explain();
    }
    M_AssertTrue(err == nullptr);
    M_AssertTrue(code == asapo::HttpCode::OK);

    consumer_impl->httpclient__->Post(args.uri_authorizer + "/v0.1/folder", "", "", &code, &err);
    M_AssertTrue(code == asapo::HttpCode::BadRequest);

    consumer_impl->httpclient__->Post(args.uri_authorizer + "/bla", "", "", &code, &err);
    M_AssertTrue(code == asapo::HttpCode::NotFound);

// check post with data
    std::string transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"aaa\"}";
    std::string cookie = "Authorization=Bearer " + folder_token + ";";
    auto content = consumer_impl->httpclient__->Post(args.uri_fts + "/v0.1/transfer", cookie, transfer, &code, &err);
    M_AssertEq("hello", content);
    M_AssertTrue(code == asapo::HttpCode::OK);
// with array
    asapo::MessageData data;
    err = consumer_impl->httpclient__->Post(args.uri_fts + "/v0.1/transfer", cookie, transfer, &data, 5, &code);
    M_AssertEq( "hello", reinterpret_cast<char const*>(data.get()));
    M_AssertTrue(code == asapo::HttpCode::OK);

    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"random\"}";
    auto io = asapo::GenerateDefaultIO();
    auto fname = args.folder + asapo::kPathSeparator + "random";
    uint64_t size = 0;
    auto expected_data = io->GetDataFromFile(fname, &size, &err);
    M_AssertEq(nullptr, err);
    err = consumer_impl->httpclient__->Post(args.uri_fts + "/v0.1/transfer", cookie, transfer, &data, size, &code);
    M_AssertTrue(code == asapo::HttpCode::OK);
    for (uint64_t i = 0; i < size; i++) {
        if (expected_data[i] != data[i]) {
            M_AssertTrue(false, "recieve array equal to sent array");
        }
    }

// with file
    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"aaa\"}";
    err = consumer_impl->httpclient__->Post(args.uri_fts + "/v0.1/transfer", cookie, transfer, "bbb", &code);
    M_AssertTrue(code == asapo::HttpCode::OK);

    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"random\"}";
    err = consumer_impl->httpclient__->Post(args.uri_fts + "/v0.1/transfer", cookie, transfer, "random", &code);
    M_AssertTrue(code == asapo::HttpCode::OK);

    return 0;
}
