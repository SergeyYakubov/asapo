#include "asapo/asapo_consumer.h"
#include <iostream>


void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << std::endl << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapo::Error err;

    auto endpoint = "localhost:8400";
    auto beamtime = "asapo_test";
    
    // test token. In production it is created during the start of the beamtime
    auto token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJl"
                 "eHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmN"
                 "DNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdG"
                 "VzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGV"
                 "zIjpbIndyaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4"
                 "t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU";

    //set it according to your configuration.
    auto path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test";

    auto credentials = asapo::SourceCredentials
            {
                asapo::SourceType::kProcessed, // should be kProcessed or kRaw, kProcessed writes to the core FS
                beamtime,                      // the folder should exist
                "",                            // can be empty or "auto", if beamtime_id is given
                "test_source",                 // source
                token                          // athorization token
            };

    auto consumer = asapo::ConsumerFactory::CreateConsumer
        (endpoint,
         path_to_files,
         true,             // True if the path_to_files is accessible locally, False otherwise
         credentials,      // same as for producer
         &err);

    exit_if_error("Cannot create consumer", err);
    consumer->SetTimeout(5000); // How long do you want to wait on non-finished stream for a message.

    // you can get info about the streams in the beamtime
    for (const auto& stream : consumer->GetStreamList("", asapo::StreamFilter::kAllStreams, &err))
    {
        std::cout << "Stream name: " << stream.name << std::endl;
        std::cout << "LastId: " << stream.last_id << std::endl;
        std::cout << "Stream finished: " << stream.finished << std::endl;
        std::cout << "Next stream: " << stream.next_stream << std::endl;
    }

    // Several consumers can use the same group_id to process messages in parallel
    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::MessageMeta mm;
    asapo::MessageData data;
    
    do {
        // GetNext is the main function to get messages from streams. You would normally call it in loop.
        // you can either manually compare the mm.id to the stream.last_id, or wait for the error to happen
        err = consumer->GetNext(group_id, &mm, &data, "default");
        
        if (err && err == asapo::ConsumerErrorTemplates::kStreamFinished) {
            // all the messages in the stream were processed
            std::cout << "stream finished" << std::endl;
            break;
        }
        
        if (err && err == asapo::ConsumerErrorTemplates::kEndOfStream) {
            // not-finished stream timeout, or wrong or empty stream
            std::cout << "stream ended" << std::endl;
            break;
        }
        
        exit_if_error("Cannot get next record", err);

        std::cout << "id: " << mm.id << std::endl;
        std::cout << "file name: " << mm.name << std::endl;
        std::cout << "message content: " << reinterpret_cast<char const*>(data.get()) << std::endl;
    } while (1);

    // you can delete the stream after consuming
    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});
    exit_if_error("Cannot delete stream", err);
    std::cout << "stream deleted" << std::endl;

    return EXIT_SUCCESS;
}
