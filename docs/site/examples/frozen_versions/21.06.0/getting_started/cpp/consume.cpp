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

    auto endpoint = "localhost:8400"; // // or your endpoint
    auto beamtime = "asapo_test";
    auto token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmNDNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbIndyaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU";

    auto path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"; //set it according to your configuration.

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, asapo::SourceCredentials{asapo::SourceType::kProcessed,beamtime, "", "test_source", token}, &err);
    exit_if_error("Cannot create consumer", err);
    consumer->SetTimeout((uint64_t) 5000);

    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::MessageMeta mm;
    asapo::MessageData data;
    err = consumer->GetNext(group_id, &mm, &data,"default");
    exit_if_error("Cannot get next record", err);

    std::cout << "id: " << mm.id << std::endl;
    std::cout << "file name: " << mm.name << std::endl;
    std::cout << "message content: " << reinterpret_cast<char const*>(data.get()) << std::endl;

// delete stream
    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});
    exit_if_error("Cannot delete stream", err);
    std::cout << "stream deleted";

    return EXIT_SUCCESS;
}
