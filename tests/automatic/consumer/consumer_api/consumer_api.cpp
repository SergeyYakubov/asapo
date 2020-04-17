#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <asapo_consumer.h>
#include "consumer/data_broker.h"
#include "testing.h"

struct Args {
    std::string server;
    std::string network_type;
    std::string run_name;
    std::string token;
    std::string datasets;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 6) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string network_type{argv[2]};
    std::string source_name{argv[3]};
    std::string token{argv[4]};
    std::string datasets{argv[5]};

    return Args{server, network_type, source_name, token, datasets};
}


void TestSingle(const std::unique_ptr<asapo::DataBroker>& broker, const std::string& group_id) {
    asapo::FileInfo fi;
    asapo::Error err;

    err = broker->GetNext(&fi, group_id, nullptr);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    M_AssertTrue(err == nullptr, "GetNext no error");
    M_AssertTrue(fi.name == "1", "GetNext filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetNext metadata");

    asapo::FileData data;
    err = broker->RetrieveData(&fi, &data);
    M_AssertTrue(err == nullptr, "RetrieveData no error");
    M_AssertEq("hello1", std::string(data.get(), data.get() + fi.size));


    err = broker->GetLast(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(fi.name == "10", "GetLast filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetLast metadata");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetNext2 error");
    auto error_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());
    M_AssertTrue(error_data->id_max == 10, "GetNext2 id max");


    err = broker->SetLastReadMarker(2, group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker no error");


    err = broker->GetById(8, &fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetById error");
    M_AssertTrue(fi.name == "8", "GetById filename");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext After GetById  no error");
    M_AssertTrue(fi.name == "3", "GetNext After GetById filename");


    err = broker->GetLast(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetLast2 no error");


    err = broker->SetLastReadMarker(8, group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker 2 no error");


    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext3 no error");
    M_AssertTrue(fi.name == "9", "GetNext3 filename");

    auto size = broker->GetCurrentSize(&err);
    M_AssertTrue(err == nullptr, "GetCurrentSize no error");
    M_AssertTrue(size == 10, "GetCurrentSize size");

    err = broker->ResetLastReadMarker(group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext4 no error");
    M_AssertTrue(fi.name == "1", "GetNext4 filename");

    auto group_id2 = broker->GenerateNewGroupId(&err);
    err = broker->GetNext(&fi, group_id2, nullptr);
    M_AssertTrue(err == nullptr, "GetNext5 no error");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");

    auto images = broker->QueryImages("meta.test = 10", &err);
    M_AssertTrue(err == nullptr, "query1");
    M_AssertTrue(images.size() == 10, "size of query answer 1");

    images = broker->QueryImages("meta.test = 10 AND name='1'", &err);
    M_AssertTrue(err == nullptr, "query2");
    M_AssertTrue(images.size() == 1, "size of query answer 2");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");


    images = broker->QueryImages("meta.test = 11", &err);
    M_AssertTrue(err == nullptr, "query3");
    M_AssertTrue(images.size() == 0, "size of query answer 3");

    images = broker->QueryImages("meta.test = 18", &err);
    M_AssertTrue(err == nullptr, "query4");
    M_AssertTrue(images.size() == 0, "size of query answer 4");

    images = broker->QueryImages("bla", &err);
    M_AssertTrue(err != nullptr, "query5");
    M_AssertTrue(images.size() == 0, "size of query answer 5");


//streams

    err = broker->GetNext(&fi, group_id, "stream1", nullptr);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }

    M_AssertTrue(err == nullptr, "GetNext stream1 no error");
    M_AssertTrue(fi.name == "11", "GetNext stream1 filename");

    err = broker->GetNext(&fi, group_id, "stream2", nullptr);
    M_AssertTrue(err == nullptr, "GetNext stream2 no error");
    M_AssertTrue(fi.name == "21", "GetNext stream2 filename");

    auto substreams = broker->GetSubstreamList(&err);
    M_AssertTrue(err == nullptr, "GetSubstreamList no error");
    M_AssertTrue(substreams.size() == 3, "substreams.size");
    M_AssertTrue(substreams[0] == "default", "substreams.name1");
    M_AssertTrue(substreams[1] == "stream1", "substreams.name2");
    M_AssertTrue(substreams[2] == "stream2", "substreams.name3");
}


void TestDataset(const std::unique_ptr<asapo::DataBroker>& broker, const std::string& group_id) {
    asapo::FileInfo fi;
    asapo::Error err;

    auto dataset = broker->GetNextDataset(group_id, &err);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    M_AssertTrue(err == nullptr, "GetNextDataSet no error");
    M_AssertTrue(dataset.content.size() == 3, "GetNextDataSet size");
    M_AssertTrue(dataset.content[0].name == "1_1", "GetNextDataSet filename");
    M_AssertTrue(dataset.content[2].name == "1_3", "GetNextDataSet filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetNext metadata");

    asapo::FileData data;
    err = broker->RetrieveData(&dataset.content[0], &data);
    M_AssertTrue(err == nullptr, "RetrieveData no error");
    M_AssertEq("hello1", std::string(data.get(), data.get() + dataset.content[0].size));


    dataset = broker->GetLastDataset(group_id, &err);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(dataset.content[0].name == "10_1", "GetLastDataset filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetLastDataset metadata");

    dataset = broker->GetNextDataset(group_id, &err);
    M_AssertTrue(err != nullptr, "GetNextDataset2 error");

    dataset = broker->GetLastDataset(group_id, &err);
    M_AssertTrue(err == nullptr, "GetLastDataset2 no error");

    dataset = broker->GetDatasetById(8, group_id, &err);
    M_AssertTrue(err == nullptr, "GetDatasetById error");
    M_AssertTrue(dataset.content[2].name == "8_3", "GetDatasetById filename");

}

void TestAll(const Args& args) {
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, ".", true,
                  asapo::SourceCredentials{args.run_name, "", "", args.token}, args.network_type, &err);
    if (err) {
        std::cout << "Error CreateServerBroker: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    broker->SetTimeout(100);
    auto group_id = broker->GenerateNewGroupId(&err);

    if (args.datasets == "single") {
        TestSingle(broker, group_id);
    }
    if (args.datasets == "dataset") {
        TestDataset(broker, group_id);
    }

}



int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    TestAll(args);
    return 0;
}
