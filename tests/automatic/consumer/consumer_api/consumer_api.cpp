#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <asapo_consumer.h>
#include "consumer/data_broker.h"
#include "testing.h"

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

    auto substreams = broker->GetSubstreamList("",&err);
    M_AssertTrue(err == nullptr, "GetSubstreamList no error");
    M_AssertTrue(substreams.size() == 3, "substreams.size");
    M_AssertTrue(substreams[0].name == "default", "substreams0.name1");
    M_AssertTrue(substreams[1].name == "stream1", "substreams1.name2");
    M_AssertTrue(substreams[2].name == "stream2", "substreams2.name3");
    std::cout<<substreams[0].Json(false)<<std::endl;
    std::cout<<substreams[1].Json(false)<<std::endl;
    std::cout<<substreams[2].Json(false)<<std::endl;
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(substreams[0].timestamp_created) == 0, "substreams0.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(substreams[0].timestamp_lastentry) == 0, "substreams0.timestamp lastentry not set");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(substreams[1].timestamp_created) == 1000, "substreams1.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(substreams[2].timestamp_created) == 2000, "substreams2.timestamp");
// acknowledges

    auto id = broker->GetLastAcknowledgedTulpeId(group_id, &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kNoData, "last ack default stream no data");
    M_AssertTrue(id == 0, "last ack default stream no data id = 0");

    auto nacks = broker->GetUnacknowledgedTupleIds(group_id, 0, 0, &err);
    M_AssertTrue(err == nullptr, "nacks default stream all");
    M_AssertTrue(nacks.size() == 10, "nacks default stream size = 10");

    err = broker->Acknowledge(group_id, 1);
    M_AssertTrue(err == nullptr, "ack default stream no error");

    nacks = broker->GetUnacknowledgedTupleIds(group_id, 0, 0, &err);
    M_AssertTrue(nacks.size() == 9, "nacks default stream size = 9 after ack");

    id = broker->GetLastAcknowledgedTulpeId(group_id, &err);
    M_AssertTrue(err == nullptr, "last ack default stream no error");
    M_AssertTrue(id == 1, "last ack default stream id = 1");

    err = broker->Acknowledge(group_id, 1, "stream1");
    M_AssertTrue(err == nullptr, "ack stream1 no error");

    nacks = broker->GetUnacknowledgedTupleIds(group_id, "stream1", 0, 0, &err);
    M_AssertTrue(nacks.size() == 4, "nacks stream1 size = 4 after ack");

// negative acks
    broker->ResetLastReadMarker(group_id);
    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextNegAckBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckBeforeResend filename");
    err = broker->NegativeAcknowledge(group_id, 1, 0);
    M_AssertTrue(err == nullptr, "NegativeAcknowledge no error");
    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextNegAckWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckWithResend filename");

// automatic resend
    broker->ResetLastReadMarker(group_id);
    broker->SetResendNacs(true, 0, 1);
    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextBeforeResend filename");

    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextWithResend filename");

    broker->SetResendNacs(false, 0, 1);
    err = broker->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextAfterResend no error");
    M_AssertTrue(fi.name == "2", "GetNextAfterResend filename");

}


void TestDataset(const std::unique_ptr<asapo::DataBroker>& broker, const std::string& group_id) {
    asapo::FileInfo fi;
    asapo::Error err;

    auto dataset = broker->GetNextDataset(group_id, 0, &err);
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


    dataset = broker->GetLastDataset(group_id, 0, &err);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(dataset.content[0].name == "10_1", "GetLastDataset filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetLastDataset metadata");

    dataset = broker->GetNextDataset(group_id, 0, &err);
    M_AssertTrue(err != nullptr, "GetNextDataset2 error");

    dataset = broker->GetLastDataset(group_id,0, &err);
    M_AssertTrue(err == nullptr, "GetLastDataset2 no error");

    dataset = broker->GetDatasetById(8, group_id, 0, &err);
    M_AssertTrue(err == nullptr, "GetDatasetById error");
    M_AssertTrue(dataset.content[2].name == "8_3", "GetDatasetById filename");

// incomplete datasets without min_size

    dataset = broker->GetNextDataset(group_id,"incomplete",0,&err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetNextDataset incomplete error");
    M_AssertTrue(dataset.content.size() == 2, "GetNextDataset incomplete size");
    M_AssertTrue(dataset.content[0].name == "1_1", "GetNextDataset incomplete filename");
    auto err_data = static_cast<const asapo::PartialErrorData*>(err->GetCustomData());
    M_AssertTrue(err_data->expected_size == 3, "GetDatasetById expected size in error");
    M_AssertTrue(err_data->id == 1, "GetDatasetById expected id in error ");
    M_AssertTrue(dataset.expected_size == 3, "GetDatasetById expected size");
    M_AssertTrue(dataset.id == 1, "GetDatasetById expected id");

    dataset = broker->GetLastDataset(group_id,"incomplete",0,&err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetLastDataset incomplete no data");

    dataset = broker->GetDatasetById(2, group_id,"incomplete", 0, &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetDatasetById incomplete error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete filename");

// incomplete datasets with min_size

    dataset = broker->GetNextDataset(group_id,"incomplete",2,&err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 2, "GetDatasetById minsize id");

    dataset = broker->GetLastDataset(group_id,"incomplete",2,&err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 5, "GetLastDataset minsize id");

    dataset = broker->GetDatasetById(2, group_id,"incomplete", 2, &err);
    M_AssertTrue(err == nullptr, "GetDatasetById incomplete minsize error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete minsize filename");


}

void TestAll(const Args& args) {
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, ".", true,
                  asapo::SourceCredentials{asapo::SourceType::kProcessed,args.run_name, "", "", args.token}, &err);
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
