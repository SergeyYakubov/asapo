#include <iostream>
#include <vector>
#include "consumer/data_broker.h"
#include "testing.h"
#include "../../../consumer/api/cpp/src/server_data_broker.h"
#include "preprocessor/definitions.h"

using asapo::M_AssertEq;
using asapo::M_AssertContains;
using asapo::M_AssertTrue;

struct Args {
    std::string uri_authorizer;
    std::string uri_fts;
    std::string folder;
    std::string fname;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string uri_authorizer{argv[1]};
    std::string uri_fts{argv[2]};
    std::string folder{argv[3]};
    std::string fname{argv[4]};
    return Args{uri_authorizer, uri_fts, folder, fname};
}


int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);
    auto token = "bnCXpOdBV90wU1zybEw1duQNSORuwaKz6oDHqmL35p0="; //token for aaa
    std::string authorize_request = "{\"Folder\":\"" + args.folder + "\",\"BeamtimeId\":\"aaa\",\"Token\":\"" + token +
                                    "\"}";
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.uri_authorizer, "", asapo::SourceCredentials{"", "", "", ""}, &err);
    auto server_broker = static_cast<asapo::ServerDataBroker*>(broker.get());

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

    asapo::FileData data;
    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, &data, 5, &code);
    M_AssertEq( "hello", reinterpret_cast<char const*>(data.get()));
    M_AssertTrue(code == asapo::HttpCode::OK);

    err = server_broker->httpclient__->Post(args.uri_fts + "/transfer", cookie, transfer, "bbb", &code);
    M_AssertTrue(code == asapo::HttpCode::OK);

    return 0;
}
