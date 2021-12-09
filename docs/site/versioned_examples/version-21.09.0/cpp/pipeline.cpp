#include "asapo/asapo_producer.h"
#include "asapo/asapo_consumer.h"
#include <iostream>

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
    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    exit_if_error("Cannot start consumer", err);
    consumer->SetTimeout(5000);
    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    // put the processed message into the new stream
    auto pipelined_stream_name = "pipelined";

    asapo::MessageMeta mm;
    asapo::MessageData data;

    do {
        // we expect the message to be in the 'default' stream already
        err = consumer->GetNext(group_id, &mm, &data, "default");

        if (err && err == asapo::ConsumerErrorTemplates::kStreamFinished) {
            std::cout << "stream finished" << std::endl;
            break;
        }

        if (err && err == asapo::ConsumerErrorTemplates::kEndOfStream) {
            std::cout << "stream ended" << std::endl;
            break;
        }
        exit_if_error("Cannot get next record", err);

        // work on our data
        auto processed_string = std::string(reinterpret_cast<char const*>(data.get())) + " processed";
        auto send_size = processed_string.size() + 1;
        auto buffer = asapo::MessageData(new uint8_t[send_size]);
        memcpy(buffer.get(), processed_string.c_str(), send_size);

        // you may use the same filename, if you want to rewrite the source file. This will result in warning, but it is a valid usecase
        asapo::MessageHeader message_header{mm.id, send_size, std::string("processed/test_file_") + std::to_string(mm.id)};
        err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, pipelined_stream_name,
                             &ProcessAfterSend);
        exit_if_error("Cannot send message", err);
    } while (1);


    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    // the meta from the last iteration corresponds to the last message
    auto last_id = mm.id;

    err = producer->SendStreamFinishedFlag("pipelined", last_id, "", &ProcessAfterSend);
    exit_if_error("Cannot finish stream", err);

    // you can remove the source stream if you do not need it anymore
    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});

    return EXIT_SUCCESS;
}
