#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "worker/data_broker.h"
#include "testing.h"

using asapo::M_AssertEq;
using asapo::M_AssertTrue;


struct Args {
    std::string server;
    std::string run_name;
    std::string token;
    std::string datasets;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string source_name{argv[2]};
    std::string token{argv[3]};
    std::string datasets{argv[4]};

    return Args{server, source_name, token, datasets};
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
    M_AssertTrue(err != nullptr, "GetNext2 error");

    err = broker->GetLast(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetLast2 no error");

    err = broker->GetById(8, &fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetById error");
    M_AssertTrue(fi.name == "8", "GetById filename");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext3 no error");
    M_AssertTrue(fi.name == "9", "GetNext3 filename");

    auto size = broker->GetNDataSets(&err);
    M_AssertTrue(err == nullptr, "GetNDataSets no error");
    M_AssertTrue(size == 10, "GetNDataSets size");

    err = broker->ResetCounter(group_id);
    M_AssertTrue(err == nullptr, "ResetCounter");

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
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, ".", args.run_name, args.token, &err);
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
