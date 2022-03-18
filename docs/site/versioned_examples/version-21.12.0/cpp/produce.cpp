#include "asapo/asapo_producer.h"
#include <iostream>

// callback snippet_start
void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err && err != asapo::ProducerErrorTemplates::kServerWarning) {
        // the data was not sent. Something is terribly wrong.
        std::cerr << "error during send: " << err << std::endl;
        return;
    } else if (err) {
        // The data was sent, but there was some unexpected problem, e.g. the file was overwritten.
        std::cout << "warning during send: " << err << std::endl;
    } else {
        // all fine
        std::cout << "successfuly send " << payload.original_header.Json() << std::endl;
        return;
    }
}
// callback snippet_end

void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
// create snippet_start
    asapo::Error err;

    auto endpoint = "localhost:8400";
    auto beamtime = "asapo_test";

    auto credentials = asapo::SourceCredentials
            {
                asapo::SourceType::kProcessed, // should be kProcessed or kRaw, kProcessed writes to the core FS
                beamtime,                      // the folder should exist
                "",                            // can be empty or "auto", if beamtime_id is given
                "test_source",                 // source
                ""                             // athorization token
            };

    auto producer = asapo::Producer::Create(endpoint,
                                            1,                               // number of threads. Increase, if the sending speed seems slow
                                            asapo::RequestHandlerType::kTcp, // Use kTcp. Use kFilesystem for direct storage of files
                                            credentials,
                                            60000,                           // timeout. Do not change.
                                            &err);
// create snippet_end
    exit_if_error("Cannot start producer", err);

// send snippet_start
    // the message must be manually copied to the buffer of the relevant size
    std::string to_send = "hello";
    auto send_size = to_send.size() + 1;
    auto buffer =  asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    // we are sending a message with with index 1. Filename must start with processed/
    asapo::MessageHeader message_header{1, send_size, "processed/test_file"};
    // use the default stream
    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
// send snippet_end
    exit_if_error("Cannot send message", err);

    // send data in loop

    // add the following at the end of the script

// finish snippet_start
    err = producer->WaitRequestsFinished(2000); // will synchronously wait for all the data to be sent.
                                                // Use it when no more data is expected.
    exit_if_error("Producer exit on timeout", err); // snippet_end_remove

    // you may want to mark the stream as finished
    err = producer->SendStreamFinishedFlag("default",          // name of the stream.
                                           1,                  // the number of the last message in the stream
                                           "",                 // next stream or empty
                                           &ProcessAfterSend);
    exit_if_error("Cannot finish stream", err); // snippet_end_remove
    std::cout << "stream finished" << std::endl;
// finish snippet_end

    return EXIT_SUCCESS;
}
