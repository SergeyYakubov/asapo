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

    auto token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJl"
                 "eHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmN"
                 "DNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdG"
                 "VzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGV"
                 "zIjpbIndyaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4"
                 "t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU";

    auto path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test";

    auto credentials = asapo::SourceCredentials{asapo::SourceType::kProcessed, beamtime, "", "test_source", token};

    auto consumer = asapo::ConsumerFactory::CreateConsumer(endpoint, path_to_files, true, credentials, &err);
    exit_if_error("Cannot create consumer", err);
    consumer->SetTimeout((uint64_t) 5000);

    auto group_id = consumer->GenerateNewGroupId(&err);
    exit_if_error("Cannot create group id", err);

    asapo::DataSet ds;
    asapo::MessageData data;

    do {
        ds = consumer->GetNextDataset(group_id, 0, "default", &err);

        if (err && err == asapo::ConsumerErrorTemplates::kStreamFinished) {
            std::cout << "stream finished" << std::endl;
            break;
        }

        if (err && err == asapo::ConsumerErrorTemplates::kEndOfStream) {
            std::cout << "stream ended" << std::endl;
            break;
        }
        exit_if_error("Cannot get next record", err);

        std::cout << "Dataset Id: " << ds.id << std::endl;

        for(int i = 0; i < ds.content.size(); i++) {
            err = consumer->RetrieveData(&ds.content[i], &data);
            exit_if_error("Cannot get dataset content", err);

            std::cout << "Part " << ds.content[i].dataset_substream << " out of " << ds.expected_size << std:: endl;
            std::cout << "message content: " << reinterpret_cast<char const*>(data.get()) << std::endl;
        }
    } while (1);

    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});
    exit_if_error("Cannot delete stream", err);

    return EXIT_SUCCESS;
}
