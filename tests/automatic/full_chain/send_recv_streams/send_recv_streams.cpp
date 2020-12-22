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

#include "asapo/asapo_consumer.h"
#include "asapo/asapo_producer.h"

using asapo::Error;
using ConsumerPtr = std::unique_ptr<asapo::Consumer>;
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

ConsumerPtr CreateConsumerAndGroup(const Args& args, Error* err) {
    auto consumer = asapo::ConsumerFactory::CreateConsumer(args.server,
                                                         ".",
                                                         true,
                                                         asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                                  args.beamtime_id, "", "", args.token},
                                                         err);
    if (*err) {
        return nullptr;
    }

    consumer->SetTimeout(10000);

    if (group_id.empty()) {
        group_id = consumer->GenerateNewGroupId(err);
        if (*err) {
            return nullptr;
        }
    }
    return consumer;
}

ProducerPtr CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, 1,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                     args.beamtime_id, "", "", args.token }, 60000, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext consumer Example", argc, argv);
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
        asapo::MessageHeader message_header{i + 1, 0, std::to_string(i + 1)};
        producer->Send(message_header, "stream1", nullptr, asapo::kTransferMetaDataOnly, ProcessAfterSend);
    }
    producer->SendStreamFinishedFlag("stream1", n, "stream2", ProcessAfterSend);
    producer->WaitRequestsFinished(10000);

    Error err;
    auto consumer = CreateConsumerAndGroup(args, &err);
    if (err) {
        std::cout << "Error CreateConsumerAndGroup: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    asapo::MessageMeta fi;
    for (uint64_t i = 0; i < n; i++) {
        consumer->GetNext(&fi, group_id, "stream1", nullptr);
    }

    err = consumer->GetNext(&fi, group_id, "stream1", nullptr);
    if (err != asapo::ConsumerErrorTemplates::kStreamFinished) {
        return 1;
    }
    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    return (err_data->next_stream == "stream2") && (files_sent == n + 1) ? 0 : 1;
}
