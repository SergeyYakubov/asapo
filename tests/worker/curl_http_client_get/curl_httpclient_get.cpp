#include <iostream>
#include <vector>
#include "worker/data_broker.h"
#include "testing.h"
#include "../../../worker/api/cpp/src/server_data_broker.h"

using hidra2::M_AssertEq;
using hidra2::M_AssertContains;

using hidra2::WorkerErrorCode;

struct Args {
    std::string uri;
    int code;
    std::string answer;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string uri{argv[1]};
    std::string answer {argv[2]};
    int code = std::stoi(argv[3]);
    return Args{uri, code, answer};
}


int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    WorkerErrorCode err;
    auto broker = hidra2::DataBrokerFactory::CreateServerBroker(args.uri, "", &err);
    auto server_broker = static_cast<hidra2::ServerDataBroker*>(broker.get());

    hidra2::HttpCode code;
    auto responce = server_broker->httpclient__->Get(args.uri, &code, &err);

    if (err != WorkerErrorCode::kOK) {
        M_AssertEq("clienterror", args.answer);
        M_AssertContains(responce, "Could");
        return 0;
    }

    M_AssertContains(responce, args.answer);
    M_AssertEq(static_cast<int>(code), args.code);

    return 0;
}
