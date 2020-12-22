#include "asapo_producer.h"

void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    if (err) {
        std::cerr << "error/warnign during send: " << err << std::endl;
        return;
    } else {
        std::cout << "successfuly send " << header.Json() << std::endl;
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

    auto source = "asapo-services2:8400";
    auto beamtime = "asapo_test";

    auto producer = asapo::Producer::Create(source, 1, asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{beamtime, "", "", ""}, 60000, &err);
    exit_if_error("Cannot start producer", err);

    std::string to_send = "hello";
    auto send_size = to_send.size() + 1;
    auto buffer =  asapo::FileData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    asapo::EventHeader event_header{1, send_size, "processed"+asapo::kPathseparator +"test_file"};
    err = producer->SendData(event_header, std::move(buffer), asapo::kDefaultIngestMode, &ProcessAfterSend);
    exit_if_error("Cannot send file", err);

    err = producer->WaitRequestsFinished(1000);
    exit_if_error("Producer exit on timeout", err);

    return EXIT_SUCCESS;
}

