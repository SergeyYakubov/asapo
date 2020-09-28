#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <mutex>
#include <string>
#include <sstream>

#include "asapo_consumer.h"
#include "asapo_producer.h"

using asapo::Error;
using BrokerPtr = std::unique_ptr<asapo::DataBroker>;
using ProducerPtr = std::unique_ptr<asapo::Producer>;
std::string group_id = "";

uint64_t files_sent;

struct Args {
    std::string server;
    std::string beamtime_id;
    std::string token;
};

void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err) {
        std::cerr << "Data was not successfully send: " << err << std::endl;
        return;
    }
    files_sent++;
}

BrokerPtr CreateBrokerAndGroup(const Args& args, Error* err) {
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, ".", true,
                  asapo::SourceCredentials{asapo::SourceType::kProcessed,args.beamtime_id, "", "", args.token}, err);
    if (*err) {
        return nullptr;
    }

    broker->SetTimeout(10000);

    if (group_id.empty()) {
        group_id = broker->GenerateNewGroupId(err);
        if (*err) {
            return nullptr;
        }
    }
    return broker;
}

ProducerPtr CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, 1,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                     args.beamtime_id, "", "", args.token }, 60, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext Broker Example", argc, argv);
    Args args;
    if (argc != 5) {
        std::cout << "Usage: " + std::string{argv[0]}
                  + " <server> <network_type> <beamtime_id> <token>"
                  <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    args.server = std::string{argv[1]};
    args.beamtime_id = std::string{argv[2]};
    args.token = std::string{argv[3]};
    auto producer = CreateProducer(args);

    uint64_t n = 1;

    for (uint64_t i = 0; i < n; i++) {
        asapo::EventHeader event_header{i + 1, 0, std::to_string(i + 1)};
        producer->SendData(event_header, "substream1", nullptr, asapo::kTransferMetaDataOnly, ProcessAfterSend);
    }
    producer->SendSubstreamFinishedFlag("substream1", n, "substream2", ProcessAfterSend);
    producer->WaitRequestsFinished(10000);

    Error err;
    auto consumer = CreateBrokerAndGroup(args, &err);
    if (err) {
        std::cout << "Error CreateBrokerAndGroup: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    asapo::FileInfo fi;
    for (uint64_t i = 0; i < n; i++) {
        consumer->GetNext(&fi, group_id, "substream1", nullptr);
    }

    err = consumer->GetNext(&fi, group_id, "substream1", nullptr);
    if (err != asapo::ConsumerErrorTemplates::kStreamFinished) {
        return 1;
    }
    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    return (err_data->next_substream == "substream2") && (files_sent == n + 1) ? 0 : 1;
}
