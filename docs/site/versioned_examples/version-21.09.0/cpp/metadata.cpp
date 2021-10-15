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
    producer->SetLogLevel(asapo::LogLevel::Error);

    // sample beamtime metadata. You can add any data you want, with any level of complexity
    // in this example we use strings and ints, and one nested structure
    auto beamtime_metadata = "{"
    "   \"name\": \"beamtime name\","
    "   \"condition\": \"beamtime condition\","
    "   \"intvalue1\": 5,"
    "   \"intvalue2\": 10,"
    "   \"structure\": {"
    "       \"structint1\": 20,"
    "       \"structint2\": 30"
    "   }"
    "}";

    // send the metadata
    // with this call the new metadata will completely replace the one that's already there
    err = producer->SendBeamtimeMetadata(beamtime_metadata, asapo::MetaIngestMode{asapo::MetaIngestOp::kReplace, true}, &ProcessAfterSend);
    exit_if_error("Cannot send metadata", err);

    // we can update the existing metadata if we want, by modifying the existing fields, or adding new ones
    auto beamtime_metadata_update = "{"
    "    \"condition\": \"updated beamtime condition\","
    "    \"newintvalue\": 15"
    "}";

    // send the metadata in the 'kUpdate' mode
    err = producer->SendBeamtimeMetadata(beamtime_metadata_update, asapo::MetaIngestMode{asapo::MetaIngestOp::kUpdate, true}, &ProcessAfterSend);
    exit_if_error("Cannot send metadata", err);

    // sample stream metadata
    auto stream_metadata = "{"
    "    \"name\": \"stream name\","
    "    \"condition\": \"stream condition\","
    "    \"intvalue\": 44"
    "}";

    // works the same way: for the initial set we use 'kReplace' the stream metadata, but update is also possible
    // update works exactly the same as for beamtime, but here we will only do 'kReplace'
    err = producer->SendStreamMetadata(stream_metadata, asapo::MetaIngestMode{asapo::MetaIngestOp::kUpdate, true}, "default", &ProcessAfterSend);
    exit_if_error("Cannot send metadata", err);

    // sample message metadata
    auto message_metadata = "{"
    "    \"name\": \"message name\","
    "    \"condition\": \"message condition\","
    "    \"somevalue\": 55"
    "}";

    std::string data_string = "hello";
    auto send_size = data_string.size() + 1;
    auto buffer = asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), data_string.c_str(), send_size);

    // the message metadata is sent together with the message itself
    // in case of datasets each part has its own metadata
    asapo::MessageHeader message_header{1, send_size, "processed/test_file", message_metadata};
    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
    exit_if_error("Cannot send message", err);

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    exit_if_error("Cannot start consumer", err);

    // read the beamtime metadata
    auto beamtime_metadata_read = consumer->GetBeamtimeMeta(&err);
    exit_if_error("Cannot get metadata", err);

    std::cout << "Updated beamtime metadata:" << std::endl << beamtime_metadata_read << std::endl;

    // read the stream metadata
    auto stream_metadata_read = consumer->GetStreamMeta("default", &err);
    exit_if_error("Cannot get metadata", err);

    std::cout << "Stream metadata:" << std::endl << stream_metadata_read << std::endl;

    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::MessageMeta mm;
    asapo::MessageData data;

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
        exit_if_error("Cannot get next record", err);

        std::cout << "Message #" << mm.id << std::endl;
        // our custom metadata is stored inside the message metadata
        std::cout << "Message metadata:" << std::endl << mm.metadata << std::endl;
    } while (1);

    return EXIT_SUCCESS;
}
