#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <asapo/asapo_producer.h>
#include "testing.h"

struct Args {
    std::string server;
    std::string source;
    std::string beamtime;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string source{argv[2]};
    std::string beamtime{argv[3]};

    return Args{server, source, beamtime};
}


void Test(const std::unique_ptr<asapo::Producer>& producer) {
    asapo::MessageMeta fi;
    asapo::Error err;

    std::string client, server;
    bool supported;
    err = producer->GetVersionInfo(&client, &server, &supported);
    M_AssertTrue(err == nullptr, "Version OK");
    M_AssertTrue(supported, "client supported by server");
    M_AssertTrue(!client.empty(), "client version");
    M_AssertTrue(!server.empty(), "server version");


    producer->GetStreamInfo("default",5000,&err);
    if (err) {
        printf("%s\n",err->Explain().c_str());
    }
    M_AssertTrue(err == nullptr, "stream info");

}


std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, 2,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed, args.beamtime,
                                                                     "", args.source, ""}, 60000, &err);
    if (err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Debug);
    return producer;
}


void TestAll(const Args& args) {
    asapo::Error err;
    auto producer = CreateProducer(args);
    if (producer==nullptr) {
        std::cout << "Error CreateProducer: " << err << std::endl;
        exit(EXIT_FAILURE);
    }
    Test(producer);
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    TestAll(args);
    return 0;
}
