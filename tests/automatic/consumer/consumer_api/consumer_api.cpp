#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <asapo/asapo_consumer.h>
#include "asapo/consumer/consumer.h"
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


void TestSingle(const std::unique_ptr<asapo::Consumer>& consumer, const std::string& group_id) {
    asapo::MessageMeta fi;
    asapo::Error err;

    err = consumer->GetNext(&fi, group_id, nullptr);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    M_AssertTrue(err == nullptr, "GetNext no error");
    M_AssertTrue(fi.name == "1", "GetNext filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetNext metadata");

    asapo::MessageData data;
    err = consumer->RetrieveData(&fi, &data);
    M_AssertTrue(err == nullptr, "RetrieveData no error");
    M_AssertEq("hello1", std::string(data.get(), data.get() + fi.size));


    err = consumer->GetLast(&fi, nullptr);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(fi.name == "10", "GetLast filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetLast metadata");

    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext2 no error");
    M_AssertTrue(fi.name == "2", "GetNext2 filename");


    err = consumer->SetLastReadMarker(2, group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker no error");


    err = consumer->GetById(8, &fi, nullptr);
    M_AssertTrue(err == nullptr, "GetById error");
    M_AssertTrue(fi.name == "8", "GetById filename");

    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext After GetById  no error");
    M_AssertTrue(fi.name == "3", "GetNext After GetById filename");


    err = consumer->GetLast(&fi, nullptr);
    M_AssertTrue(err == nullptr, "GetLast2 no error");


    err = consumer->SetLastReadMarker(8, group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker 2 no error");


    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext3 no error");
    M_AssertTrue(fi.name == "9", "GetNext3 filename");

    auto size = consumer->GetCurrentSize(&err);
    M_AssertTrue(err == nullptr, "GetCurrentSize no error");
    M_AssertTrue(size == 10, "GetCurrentSize size");

    err = consumer->ResetLastReadMarker(group_id);
    M_AssertTrue(err == nullptr, "SetLastReadMarker");

    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNext4 no error");
    M_AssertTrue(fi.name == "1", "GetNext4 filename");

    auto group_id2 = consumer->GenerateNewGroupId(&err);
    err = consumer->GetNext(&fi, group_id2, nullptr);
    M_AssertTrue(err == nullptr, "GetNext5 no error");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");

    auto messages = consumer->QueryMessages("meta.test = 10", &err);
    M_AssertTrue(err == nullptr, "query1");
    M_AssertTrue(messages.size() == 10, "size of query answer 1");

    messages = consumer->QueryMessages("meta.test = 10 AND name='1'", &err);
    M_AssertTrue(err == nullptr, "query2");
    M_AssertTrue(messages.size() == 1, "size of query answer 2");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");


    messages = consumer->QueryMessages("meta.test = 11", &err);
    M_AssertTrue(err == nullptr, "query3");
    M_AssertTrue(messages.size() == 0, "size of query answer 3");

    messages = consumer->QueryMessages("meta.test = 18", &err);
    M_AssertTrue(err == nullptr, "query4");
    M_AssertTrue(messages.size() == 0, "size of query answer 4");

    messages = consumer->QueryMessages("bla", &err);
    M_AssertTrue(err != nullptr, "query5");
    M_AssertTrue(messages.size() == 0, "size of query answer 5");


//streams

    err = consumer->GetNext(&fi, group_id, "stream1", nullptr);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }

    M_AssertTrue(err == nullptr, "GetNext stream1 no error");
    M_AssertTrue(fi.name == "11", "GetNext stream1 filename");

    err = consumer->GetNext(&fi, group_id, "stream2", nullptr);
    M_AssertTrue(err == nullptr, "GetNext stream2 no error");
    M_AssertTrue(fi.name == "21", "GetNext stream2 filename");

    auto streams = consumer->GetStreamList("",&err);
    M_AssertTrue(err == nullptr, "GetStreamList no error");
    M_AssertTrue(streams.size() == 3, "streams.size");
    M_AssertTrue(streams[0].name == "default", "streams0.name1");
    M_AssertTrue(streams[1].name == "stream1", "streams1.name2");
    M_AssertTrue(streams[2].name == "stream2", "streams2.name3");
    std::cout<<streams[0].Json(false)<<std::endl;
    std::cout<<streams[1].Json(false)<<std::endl;
    std::cout<<streams[2].Json(false)<<std::endl;
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[0].timestamp_created) == 0, "streams0.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[0].timestamp_lastentry) == 0, "streams0.timestamp lastentry not set");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[1].timestamp_created) == 1000, "streams1.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[2].timestamp_created) == 2000, "streams2.timestamp");
// acknowledges

    auto id = consumer->GetLastAcknowledgedTulpeId(group_id, &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kNoData, "last ack default stream no data");
    M_AssertTrue(id == 0, "last ack default stream no data id = 0");

    auto nacks = consumer->GetUnacknowledgedTupleIds(group_id, 0, 0, &err);
    M_AssertTrue(err == nullptr, "nacks default stream all");
    M_AssertTrue(nacks.size() == 10, "nacks default stream size = 10");

    err = consumer->Acknowledge(group_id, 1);
    M_AssertTrue(err == nullptr, "ack default stream no error");

    nacks = consumer->GetUnacknowledgedTupleIds(group_id, 0, 0, &err);
    M_AssertTrue(nacks.size() == 9, "nacks default stream size = 9 after ack");

    id = consumer->GetLastAcknowledgedTulpeId(group_id, &err);
    M_AssertTrue(err == nullptr, "last ack default stream no error");
    M_AssertTrue(id == 1, "last ack default stream id = 1");

    err = consumer->Acknowledge(group_id, 1, "stream1");
    M_AssertTrue(err == nullptr, "ack stream1 no error");

    nacks = consumer->GetUnacknowledgedTupleIds(group_id, "stream1", 0, 0, &err);
    M_AssertTrue(nacks.size() == 4, "nacks stream1 size = 4 after ack");

// negative acks
    consumer->ResetLastReadMarker(group_id);
    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextNegAckBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckBeforeResend filename");
    err = consumer->NegativeAcknowledge(group_id, 1, 0);
    M_AssertTrue(err == nullptr, "NegativeAcknowledge no error");
    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextNegAckWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckWithResend filename");

// automatic resend
    consumer->ResetLastReadMarker(group_id);
    consumer->SetResendNacs(true, 0, 1);
    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextBeforeResend filename");

    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextWithResend filename");

    consumer->SetResendNacs(false, 0, 1);
    err = consumer->GetNext(&fi, group_id, nullptr);
    M_AssertTrue(err == nullptr, "GetNextAfterResend no error");
    M_AssertTrue(fi.name == "2", "GetNextAfterResend filename");

}


void TestDataset(const std::unique_ptr<asapo::Consumer>& consumer, const std::string& group_id) {
    asapo::MessageMeta fi;
    asapo::Error err;

    auto dataset = consumer->GetNextDataset(group_id, 0, &err);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    M_AssertTrue(err == nullptr, "GetNextDataSet no error");
    M_AssertTrue(dataset.content.size() == 3, "GetNextDataSet size");
    M_AssertTrue(dataset.content[0].name == "1_1", "GetNextDataSet filename");
    M_AssertTrue(dataset.content[2].name == "1_3", "GetNextDataSet filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetNext metadata");

    asapo::MessageData data;
    err = consumer->RetrieveData(&dataset.content[0], &data);
    M_AssertTrue(err == nullptr, "RetrieveData no error");
    M_AssertEq("hello1", std::string(data.get(), data.get() + dataset.content[0].size));


    dataset = consumer->GetLastDataset(0, &err);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(dataset.content[0].name == "10_1", "GetLastDataset filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetLastDataset metadata");

    dataset = consumer->GetNextDataset(group_id, 0, &err);
    M_AssertTrue(err == nullptr, "GetNextDataset2 no error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetNextDataSet2 filename");

    dataset = consumer->GetLastDataset(0, &err);
    M_AssertTrue(err == nullptr, "GetLastDataset2 no error");

    dataset = consumer->GetDatasetById(8, 0, &err);
    M_AssertTrue(err == nullptr, "GetDatasetById error");
    M_AssertTrue(dataset.content[2].name == "8_3", "GetDatasetById filename");

// incomplete datasets without min_size

    dataset = consumer->GetNextDataset(group_id,"incomplete",0,&err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetNextDataset incomplete error");
    M_AssertTrue(dataset.content.size() == 2, "GetNextDataset incomplete size");
    M_AssertTrue(dataset.content[0].name == "1_1", "GetNextDataset incomplete filename");
    auto err_data = static_cast<const asapo::PartialErrorData*>(err->GetCustomData());
    M_AssertTrue(err_data->expected_size == 3, "GetDatasetById expected size in error");
    M_AssertTrue(err_data->id == 1, "GetDatasetById expected id in error ");
    M_AssertTrue(dataset.expected_size == 3, "GetDatasetById expected size");
    M_AssertTrue(dataset.id == 1, "GetDatasetById expected id");

    dataset = consumer->GetLastDataset("incomplete", 0, &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetLastDataset incomplete no data");

    dataset = consumer->GetDatasetById(2, "incomplete", 0, &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetDatasetById incomplete error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete filename");

// incomplete datasets with min_size

    dataset = consumer->GetNextDataset(group_id,"incomplete",2,&err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 2, "GetDatasetById minsize id");

    dataset = consumer->GetLastDataset("incomplete", 2, &err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 5, "GetLastDataset minsize id");

    dataset = consumer->GetDatasetById(2, "incomplete", 2, &err);
    M_AssertTrue(err == nullptr, "GetDatasetById incomplete minsize error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete minsize filename");


}

void TestAll(const Args& args) {
    asapo::Error err;
    auto consumer = asapo::ConsumerFactory::CreateConsumer(args.server,
                                                         ".",
                                                         true,
                                                         asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                                  args.run_name, "", "", args.token},
                                                         &err);
    if (err) {
        std::cout << "Error CreateConsumer: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    consumer->SetTimeout(100);
    auto group_id = consumer->GenerateNewGroupId(&err);

    if (args.datasets == "single") {
        TestSingle(consumer, group_id);
    }
    if (args.datasets == "dataset") {
        TestDataset(consumer, group_id);
    }

}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    TestAll(args);
    return 0;
}
