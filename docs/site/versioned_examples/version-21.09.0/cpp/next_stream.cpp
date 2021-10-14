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

    // let's start with producing a sample of 10 simple messages
    for (uint64_t i = 1; i <= 10; i++) {
        std::string to_send = "content of the message #" + std::to_string(i);
        auto send_size = to_send.size() + 1;
        auto buffer =  asapo::MessageData(new uint8_t[send_size]);
        memcpy(buffer.get(), to_send.c_str(), send_size);

        asapo::MessageHeader message_header{i, send_size, "processed/test_file_" + std::to_string(i)};
        err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
        exit_if_error("Cannot send message", err);
    }

    // finish the stream and set the next stream to be called 'next'
    producer->SendStreamFinishedFlag("default", 10, "next", &ProcessAfterSend);

    // populate the 'next' stream as well
    for (uint64_t i = 1; i <= 5; i++) {
        std::string to_send = "content of the message #" + std::to_string(i);
        auto send_size = to_send.size() + 1;
        auto buffer =  asapo::MessageData(new uint8_t[send_size]);
        memcpy(buffer.get(), to_send.c_str(), send_size);

        asapo::MessageHeader message_header{i, send_size, "processed/test_file_next_" + std::to_string(i)};
        err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "next", &ProcessAfterSend);
        exit_if_error("Cannot send message", err);
    }

    // we leave the 'next' stream unfinished, but the chain of streams can be of any length

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    consumer->SetTimeout(5000);
    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::MessageMeta mm;
    asapo::MessageData data;

    // we start with the 'default' stream (the first one)
    std::string stream_name = "default";

    do {
        err = consumer->GetNext(group_id, &mm, &data, stream_name);

        if (err && err == asapo::ConsumerErrorTemplates::kStreamFinished) {
            // when the stream finishes, we look for the info on the next stream
            auto streams = consumer->GetStreamList("", asapo::StreamFilter::kAllStreams, &err);
            // first, we find the stream with our name in the list of streams
            auto stream = std::find_if(streams.begin(), streams.end(), [&stream_name](const asapo::StreamInfo& s) { return s.name == stream_name; });

            // then we look if the field 'nextStream' is set and not empty
            if (stream != streams.end() && !stream->next_stream.empty()) {
                // if it's not, we continue with the next stream
                stream_name = stream->next_stream;
                std::cout << "Changing stream to the next one: " << stream_name << std::endl;
                continue;
            } else {
                // otherwise we stop
                std::cout << "stream finished" << std::endl;
                break;
            }
        }

        if (err && err == asapo::ConsumerErrorTemplates::kEndOfStream) {
            std::cout << "stream ended" << std::endl;
            break;
        }
        exit_if_error("Cannot get next record", err);

        std::cout << "Message #" << mm.id << ", message content: " << reinterpret_cast<char const*>(data.get()) << std::endl;
    } while (1);

   return EXIT_SUCCESS;
}
