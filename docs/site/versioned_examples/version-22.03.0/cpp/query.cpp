#include "asapo/asapo_producer.h"
#include "asapo/asapo_consumer.h"
#include <iostream>
#include <chrono>

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

void PrintMessages(asapo::MessageMetas metas, std::unique_ptr<asapo::Consumer>& consumer) {
    asapo::MessageData data;
    asapo::Error err;
    for (int i = 0; i < metas.size(); i++) {
        err = consumer->RetrieveData(&metas[i], &data);
        std::cout << "Message #" << metas[i].id
                  << ", content: " << reinterpret_cast<char const*>(data.get())
                  << ", user metadata: " << metas[i].metadata << std::endl;
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

    err = producer->DeleteStream("default", 0, asapo::DeleteStreamOptions{true, true});
    exit_if_error("Cannot delete stream", err);

    // let's start with producing some messages with metadata
    for (uint64_t i = 1; i <= 10; i++) {
        auto message_metadata = "{"
        "    \"condition\": \"condition #" + std::to_string(i) + "\","
        "    \"somevalue\": " + std::to_string(i * 10) +
        "}";

        std::string to_send = "message#" + std::to_string(i);
        auto send_size = to_send.size() + 1;
        auto buffer =  asapo::MessageData(new uint8_t[send_size]);
        memcpy(buffer.get(), to_send.c_str(), send_size);

        asapo::MessageHeader message_header{i, send_size, "processed/test_file_" + std::to_string(i), message_metadata};
        err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
        exit_if_error("Cannot send message", err);
    }

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    exit_if_error("Cannot create group id", err);
    consumer->SetTimeout(5000);

    // by_id snippet_start
    // simple query, same as GetById
    auto metadatas = consumer->QueryMessages("_id = 1", "default", &err);
    // by_id snippet_end
    exit_if_error("Cannot query messages", err);
    std::cout << "Message with ID = 1" << std::endl;
    PrintMessages(metadatas, consumer);

    // by_ids snippet_start
    // the query that requests the range of IDs
    metadatas = consumer->QueryMessages("_id >= 8", "default", &err);
    // by_ids snippet_end
    exit_if_error("Cannot query messages", err);
    std::cout << "essages with ID >= 8" << std::endl;
    PrintMessages(metadatas, consumer);

    // string_equal snippet_start
    // the query that has some specific requirement for message metadata
    metadatas = consumer->QueryMessages("meta.condition = \"condition #7\"", "default", &err);
    // string_equal snippet_end
    exit_if_error("Cannot query messages", err);
    std::cout << "Message with condition = 'condition #7'" << std::endl;
    PrintMessages(metadatas, consumer);

    // int_compare snippet_start
    // the query that has several requirements for user metadata
    metadatas = consumer->QueryMessages("meta.somevalue > 30 AND meta.somevalue < 60", "default", &err);
    // int_compare snippet_end
    exit_if_error("Cannot query messages", err);
    std::cout << "Message with 30 < somevalue < 60" << std::endl;
    PrintMessages(metadatas, consumer);

    // timestamp snippet_start
    // the query that is based on the message's timestamp
    auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto fifteen_minutes_ago = std::chrono::duration_cast<std::chrono::nanoseconds>((std::chrono::system_clock::now() - std::chrono::minutes(15)).time_since_epoch()).count();
    metadatas = consumer->QueryMessages("timestamp < " + std::to_string(now) + " AND timestamp > " + std::to_string(fifteen_minutes_ago), "default", &err);
    // timestamp snippet_end
    exit_if_error("Cannot query messages", err);
    std::cout << "Messages in the last 15 minutes" << std::endl;
    PrintMessages(metadatas, consumer);

    return EXIT_SUCCESS;
}
