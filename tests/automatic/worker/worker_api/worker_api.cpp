#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "worker/data_broker.h"
#include "testing.h"

using asapo::M_AssertEq;
using asapo::M_AssertTrue;


struct Args {
    std::string server;
    std::string run_name;
    std::string token;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string source_name{argv[2]};
    std::string token{argv[3]};

    return Args{server, source_name, token};
}

void GetAllFromBroker(const Args& args) {
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, "dummy", args.run_name, args.token, &err);
    broker->SetTimeout(100);
    auto group_id = broker->GenerateNewGroupId(&err);
    asapo::FileInfo fi;
    err = broker->GetNext(&fi, group_id, nullptr);
    if (err) {
        std::cout<<err->Explain()<<std::endl;
    }
    M_AssertTrue(err == nullptr, "GetNext no error");
    M_AssertTrue(fi.name == "1", "GetNext filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetNext metadata");

    err = broker->GetLast(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(fi.name == "10", "GetLast filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetLast metadata");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err != nullptr, "GetNext2 error");

    err = broker->GetLast(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetLast2 no error");

    err = broker->GetById(8, &fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetById error");
    M_AssertTrue(fi.name == "8", "GetById filename");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext3 no error");
    M_AssertTrue(fi.name == "9", "GetNext3 filename");

    auto size = broker->GetNDataSets(&err);
    M_AssertTrue(err == nullptr, "GetNDataSets no error");
    M_AssertTrue(size == 10, "GetNDataSets size");

    err = broker->ResetCounter(group_id);
    M_AssertTrue(err == nullptr, "ResetCounter");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext4 no error");
    M_AssertTrue(fi.name == "1", "GetNext4 filename");

    auto group_id2 = broker->GenerateNewGroupId(&err);
    err = broker->GetNext(&fi, group_id2, nullptr);
    M_AssertTrue(err == nullptr, "GetNext5 no error");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");

}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    GetAllFromBroker(args);
    return 0;
}
