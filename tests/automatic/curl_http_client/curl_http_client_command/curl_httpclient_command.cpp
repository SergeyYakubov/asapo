#include <iostream>
#include <vector>
#include "consumer/data_broker.h"
#include "testing.h"
#include "../../../consumer/api/cpp/src/server_data_broker.h"
#include "preprocessor/definitions.h"
#include "io/io_factory.h"
#include "io/io.h"

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
    auto token = "bnCXpOdBV90wU1zybEw1duQNSORuwaKz6oDHqmL35p0="; //token for aaa
    std::string authorize_request = "{\"Folder\":\"" + args.folder + "\",\"BeamtimeId\":\"aaa\",\"Token\":\"" + token +
                                    "\"}";
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.uri_authorizer, "", true, asapo::SourceCredentials{asapo::SourceType::kProcessed,"", "", "", ""}, &err);
    auto server_broker = static_cast<asapo::ServerDataBroker*>(broker.get());
    M_AssertEq(nullptr, err);

    asapo::HttpCode code;
    std::string response;
    std::string input_data;
    auto folder_token = server_broker->httpclient__->Post(args.uri_authorizer + "/folder", "", authorize_request, &code,
                        &err);
    M_AssertTrue(err == nullptr);
    M_AssertTrue(code == asapo::HttpCode::OK);
    if (err) {
        std::cout << err->Explain();
    }

    server_broker->httpclient__->Post(args.uri_authorizer + "/folder", "", "", &code, &err);
    M_AssertTrue(code == asapo::HttpCode::BadRequest);

    server_broker->httpclient__->Post(args.uri_authorizer + "/bla", "", "", &code, &err);
    M_AssertTrue(code == asapo::HttpCode::NotFound);

// check post with data
    std::string transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"aaa\"}";
    std::string cookie = "Authorization=Bearer " + folder_token + ";";
    auto content = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, &code, &err);
    M_AssertEq("hello", content);
    M_AssertTrue(code == asapo::HttpCode::OK);
// with array
    asapo::FileData data;
    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, &data, 5, &code);
    M_AssertEq( "hello", reinterpret_cast<char const*>(data.get()));
    M_AssertTrue(code == asapo::HttpCode::OK);

    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"random\"}";
    auto io = asapo::GenerateDefaultIO();
    auto fname = args.folder + asapo::kPathSeparator + "random";
    uint64_t size = 0;
    auto expected_data = io->GetDataFromFile(fname, &size, &err);
    M_AssertEq(nullptr, err);
    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, &data, size, &code);
    M_AssertTrue(code == asapo::HttpCode::OK);
    for (uint64_t i = 0; i < size; i++) {
        if (expected_data[i] != data[i]) {
            M_AssertTrue(false, "recieve array equal to sent array");
        }
    }

// with file
    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"aaa\"}";
    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, "bbb", &code);
    M_AssertTrue(code == asapo::HttpCode::OK);

    transfer = "{\"Folder\":\"" + args.folder + "\",\"FileName\":\"random\"}";
    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, "random", &code);
    M_AssertTrue(code == asapo::HttpCode::OK);

    return 0;
}
