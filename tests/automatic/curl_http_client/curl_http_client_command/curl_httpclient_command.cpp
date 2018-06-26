#include <iostream>
#include <vector>
#include "worker/data_broker.h"
#include "testing.h"
#include "../../../worker/api/cpp/src/server_data_broker.h"

using asapo::M_AssertEq;
using asapo::M_AssertContains;

struct Args {
    std::string command;
    std::string uri;
    int code;
    std::string answer;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string command{argv[1]};
    std::string uri{argv[2]};
    std::string answer {argv[3]};
    int code = std::stoi(argv[4]);
    return Args{command, uri, code, answer};
}


int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.uri, "", "", &err);
    auto server_broker = static_cast<asapo::ServerDataBroker*>(broker.get());

    asapo::HttpCode code;
    std::string response;
    if (args.command == "GET") {
        response = server_broker->httpclient__->Get(args.uri, &code, &err);
    } else if  (args.command == "POST") {
        response = server_broker->httpclient__->Post(args.uri, "testdata", &code, &err);
    }

    if (err != nullptr) {
        M_AssertEq("clienterror", args.answer);
        M_AssertContains(response, "Could");
        return 0;
    }

    M_AssertContains(response, args.answer);
    M_AssertEq(static_cast<int>(code), args.code);

    return 0;
}
