#include "asapo/asapo_producer.h"
#include "asapo/asapo_consumer.h"
#include <iostream>
#include <set>

void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err && err != asapo::ProducerErrorTemplates::kServerWarning) {
        std::cerr << "error during send: " << err << std::endl;
        return;
    } else if (err) {
        std::cout << "warning during send: " << err << std::endl;
    } else {
        std::cout << "successfuly send " << payload.original_header.Json() << std::endl;
        return;
    }
}

void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapo::Error err;

    auto endpoint = "localhost:8400";
    auto beamtime = "asapo_test";

    auto token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJl"
                 "eHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmN"
                 "DNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdG"
                 "VzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGV"
                 "zIjpbIndyaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4"
                 "t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU";

    auto path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test";

    auto credentials = asapo::SourceCredentials{asapo::SourceType::kProcessed, beamtime, "", "test_source", token};

    auto producer = asapo::Producer::Create(endpoint, 1, asapo::RequestHandlerType::kTcp, credentials, 60000, &err);
    exit_if_error("Cannot start producer", err);

    producer->SetLogLevel(asapo::LogLevel::Error);

    err = producer->DeleteStream("default", 1000, asapo::DeleteStreamOptions{true, true});
    exit_if_error("Cannot delete stream", err);

    // let's start with producing a sample of 10 simple messages
    for (uint64_t i = 1; i <= 10; i++) {
        std::string to_send = "message#" + std::to_string(i);
        auto send_size = to_send.size() + 1;
        auto buffer =  asapo::MessageData(new uint8_t[send_size]);
        memcpy(buffer.get(), to_send.c_str(), send_size);

        asapo::MessageHeader message_header{i, send_size, "processed/test_file_" + std::to_string(i)};
        err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
        exit_if_error("Cannot send message", err);
    }

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    exit_if_error("Cannot start consumer", err);
    consumer->SetTimeout(5000);
    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    // consume snippet_start
    asapo::MessageMeta mm;
    asapo::MessageData data;

    const std::set<int> ids {3, 5, 7};

    // the flag to separate the first attempt for message #3
    bool firstTryNegative = true;

    do {
        err = consumer->GetNext(group_id, &mm, &data, "default");

        if (err && err == asapo::ConsumerErrorTemplates::kStreamFinished) {
            std::cout << "stream finished" << std::endl;
            break;
        }

        if (err && err == asapo::ConsumerErrorTemplates::kEndOfStream) {
            std::cout << "stream ended" << std::endl;
            break;
        }
        exit_if_error("Cannot get next record", err); // snippet_end_remove

        // acknowledge all the messages except the ones in the set
        if (ids.find(mm.id) == ids.end()) {
            std::cout << "Acknowledge the message #" << mm.id << std::endl;
            consumer->Acknowledge(group_id, mm.id, "default");
        }

        // for message #3 we issue a negative acknowledgement, which will put it at the next place in the stream
        // in this case, it will be put in the end of a stream
        if (mm.id == 3) {
            if (firstTryNegative) {
                std::cout << "Negative acknowledgement of the message #" << mm.id << std::endl;
                // make the acknowledgement with a delay of 1 second
                consumer->NegativeAcknowledge(group_id, mm.id, 2000, "default");
                firstTryNegative = false;
            } else {
                // on our second attempt we acknowledge the message
                std::cout << "Second try of the message #" << mm.id << std::endl;
                consumer->Acknowledge(group_id, mm.id, "default");
            }
        }
    } while (1);
    // consume snippet_end

    // print snippet_start
    auto unacknowledgedMessages = consumer->GetUnacknowledgedMessages(group_id, 0, 0, "default", &err);
    exit_if_error("Could not get list of messages", err); // snippet_end_remove

    for (int i = 0; i < unacknowledgedMessages.size(); i++) {
        err = consumer->GetById(unacknowledgedMessages[i], &mm, &data, "default");
        exit_if_error("Cannot get message", err); // snippet_end_remove

        std::cout << "Unacknowledged message: " << reinterpret_cast<char const*>(data.get()) << std::endl;
        std::cout << "id: " << mm.id << std::endl;
        std::cout << "file name: " << mm.name << std::endl;
    }
    // print snippet_end

   return EXIT_SUCCESS;
}
