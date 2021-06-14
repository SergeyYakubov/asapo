#include "asapo/asapo_consumer.h"
#include <iostream>

void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapo::Error err;

    auto endpoint = "asapo-services2:8400";
    auto beamtime = "asapo_test";
    auto token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc=";

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, "", true, asapo::SourceCredentials{asapo::SourceType::kProcessed, beamtime, "", "", token}, &err);
    exit_if_error("Cannot create consumer", err);
    consumer->SetTimeout((uint64_t) 1000);

    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::MessageMeta fi;
    asapo::MessageData data;

    err = consumer->GetLast(&fi, &data, group_id);
    exit_if_error("Cannot get next record", err);

    std::cout << "id: " << fi.id << std::endl;
    std::cout << "file name: " << fi.name << std::endl;
    std::cout << "file content: " << reinterpret_cast<char const*>(data.get()) << std::endl;
    return EXIT_SUCCESS;
}

