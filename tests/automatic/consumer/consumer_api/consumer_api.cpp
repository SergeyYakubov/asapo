#include <iostream>
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

    std::string client, server;
    bool supported;
    err = consumer->GetVersionInfo(&client, &server, &supported);
    M_AssertTrue(err == nullptr, "Version OK");
    M_AssertTrue(supported, "client supported by server");
    M_AssertTrue(!client.empty(), "client version");
    M_AssertTrue(!server.empty(), "server version");

    err = consumer->GetNext(group_id, &fi, nullptr, "default");
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


    err = consumer->GetLast(&fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(fi.name == "10", "GetLast filename");
    M_AssertTrue(fi.metadata == "{\"test\":10}", "GetLast metadata");

    err = consumer->GetLast(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetLast inside group no error");
    M_AssertTrue(fi.name == "10", "GetLast inside group filename");

    err = consumer->GetLast(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetLast inside group error second time");

    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNext2 no error");
    M_AssertTrue(fi.name == "2", "GetNext2 filename");


    err = consumer->SetLastReadMarker(group_id, 2, "default");
    M_AssertTrue(err == nullptr, "SetLastReadMarker no error");


    err = consumer->GetById(8, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetById error");
    M_AssertTrue(fi.name == "8", "GetById filename");

    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNext After GetById  no error");
    M_AssertTrue(fi.name == "3", "GetNext After GetById filename");


    err = consumer->GetLast(&fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetLast2 no error");


    err = consumer->SetLastReadMarker(group_id, 8, "default");
    M_AssertTrue(err == nullptr, "SetLastReadMarker 2 no error");


    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNext3 no error");
    M_AssertTrue(fi.name == "9", "GetNext3 filename");

    auto size = consumer->GetCurrentSize("default", &err);
    M_AssertTrue(err == nullptr, "GetCurrentSize no error");
    M_AssertTrue(size == 10, "GetCurrentSize size");

    auto size1 = consumer->GetCurrentSize("stream1", &err);
    M_AssertTrue(err == nullptr, "GetCurrentSize 1 no error");
    M_AssertTrue(size1 == 5, "GetCurrentSize 1 size");

    auto size2 = consumer->GetCurrentSize("stream2", &err);
    M_AssertTrue(err == nullptr, "GetCurrentSize 2 no error");
    M_AssertTrue(size2 == 5, "GetCurrentSize 2 size");

    err = consumer->ResetLastReadMarker(group_id, "default");
    M_AssertTrue(err == nullptr, "SetLastReadMarker");

    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNext4 no error");
    M_AssertTrue(fi.name == "1", "GetNext4 filename");

    auto group_id2 = consumer->GenerateNewGroupId(&err);
    err = consumer->GetNext(group_id2, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNext5 no error");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");

    auto messages = consumer->QueryMessages("meta.test = 10", "default", &err);
    M_AssertTrue(err == nullptr, "query1");
    M_AssertTrue(messages.size() == 10, "size of query answer 1");

    messages = consumer->QueryMessages("meta.test = 10 AND name='1'", "default", &err);
    M_AssertTrue(err == nullptr, "query2");
    M_AssertTrue(messages.size() == 1, "size of query answer 2");
    M_AssertTrue(fi.name == "1", "GetNext5  filename");


    messages = consumer->QueryMessages("meta.test = 11", "default", &err);
    M_AssertTrue(err == nullptr, "query3");
    M_AssertTrue(messages.size() == 0, "size of query answer 3");

    messages = consumer->QueryMessages("meta.test = 18", "default", &err);
    M_AssertTrue(err == nullptr, "query4");
    M_AssertTrue(messages.size() == 0, "size of query answer 4");

    messages = consumer->QueryMessages("bla", "default", &err);
    M_AssertTrue(err != nullptr, "query5");
    M_AssertTrue(messages.size() == 0, "size of query answer 5");

//streams

    err = consumer->GetNext(group_id, &fi, nullptr, "stream1");
    if (err) {
        std::cout << err->Explain() << std::endl;
    }

    M_AssertTrue(err == nullptr, "GetNext stream1 no error");
    M_AssertTrue(fi.name == "11", "GetNext stream1 filename");

    err = consumer->GetNext(group_id, &fi, nullptr, "stream2");
    M_AssertTrue(err == nullptr, "GetNext stream2 no error");
    M_AssertTrue(fi.name == "21", "GetNext stream2 filename");

    auto streams = consumer->GetStreamList("", asapo::StreamFilter::kAllStreams, &err);
    M_AssertTrue(err == nullptr, "GetStreamList no error");
    M_AssertTrue(streams.size() == 3, "streams.size");
    M_AssertTrue(streams[0].name == "default", "streams0.name");
    M_AssertTrue(streams[1].name == "stream1", "streams1.name");
    M_AssertTrue(streams[2].name == "stream2", "streams2.name");
    M_AssertTrue(streams[1].finished, "stream1 finished");
    M_AssertTrue(streams[1].next_stream == "ns", "stream1 next stream");
    M_AssertTrue(streams[2].finished, "stream2 finished");
    M_AssertTrue(streams[2].next_stream == "", "stream2 no next stream");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[0].timestamp_created) == 0, "streams0.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[0].timestamp_lastentry) == 0, "streams0.timestamp lastentry");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[1].timestamp_created) == 1000, "streams1.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[1].timestamp_lastentry) == 1000, "streams1.timestamp lastentry");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[2].timestamp_created) == 2000, "streams2.timestamp");
    M_AssertTrue(asapo::NanosecsEpochFromTimePoint(streams[2].timestamp_lastentry) == 2000, "streams2.timestamp lastentry");
// acknowledges

    auto id = consumer->GetLastAcknowledgedMessage(group_id, "default", &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kNoData, "last ack default stream no data");
    M_AssertTrue(id == 0, "last ack default stream no data id = 0");

    auto nacks = consumer->GetUnacknowledgedMessages(group_id, 0, 0, "default", &err);
    M_AssertTrue(err == nullptr, "nacks default stream all");
    M_AssertTrue(nacks.size() == 10, "nacks default stream size = 10");

    err = consumer->Acknowledge(group_id, 1, "default");
    M_AssertTrue(err == nullptr, "ack default stream no error");

    nacks = consumer->GetUnacknowledgedMessages(group_id, 0, 0, "default", &err);
    M_AssertTrue(nacks.size() == 9, "nacks default stream size = 9 after ack");

    id = consumer->GetLastAcknowledgedMessage(group_id, "default", &err);
    M_AssertTrue(err == nullptr, "last ack default stream no error");
    M_AssertTrue(id == 1, "last ack default stream id = 1");

    err = consumer->Acknowledge(group_id, 1, "stream1");
    M_AssertTrue(err == nullptr, "ack stream1 no error");

    nacks = consumer->GetUnacknowledgedMessages(group_id, 0, 0, "stream1", &err);
    M_AssertTrue(nacks.size() == 4, "nacks stream1 size = 4 after ack");

// negative acks
    consumer->ResetLastReadMarker(group_id, "default");
    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNextNegAckBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckBeforeResend filename");
    err = consumer->NegativeAcknowledge(group_id, 1, 0, "default");
    M_AssertTrue(err == nullptr, "NegativeAcknowledge no error");
    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNextNegAckWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextNegAckWithResend filename");

// automatic resend
    consumer->ResetLastReadMarker(group_id, "default");
    consumer->SetResendNacs(true, 0, 1);
    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNextBeforeResend no error");
    M_AssertTrue(fi.name == "1", "GetNextBeforeResend filename");

    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNextWithResend no error");
    M_AssertTrue(fi.name == "1", "GetNextWithResend filename");

    consumer->SetResendNacs(false, 0, 1);
    err = consumer->GetNext(group_id, &fi, nullptr, "default");
    M_AssertTrue(err == nullptr, "GetNextAfterResend no error");
    M_AssertTrue(fi.name == "2", "GetNextAfterResend filename");

// delete stream

    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});
    M_AssertTrue(err == nullptr, "delete default stream ok");
    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, true});
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kWrongInput, "delete non existing stream error");
    err = consumer->DeleteStream("default", asapo::DeleteStreamOptions{true, false});
    M_AssertTrue(err == nullptr, "delete non existing stream ok");

}


void TestDataset(const std::unique_ptr<asapo::Consumer>& consumer, const std::string& group_id) {
    asapo::MessageMeta fi;
    asapo::Error err;

    auto dataset = consumer->GetNextDataset(group_id, 0, "default", &err);
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


    dataset = consumer->GetLastDataset(0, "default", &err);
    M_AssertTrue(err == nullptr, "GetLast no error");
    M_AssertTrue(dataset.content[0].name == "10_1", "GetLastDataset filename");
    M_AssertTrue(dataset.content[0].metadata == "{\"test\":10}", "GetLastDataset metadata");

    consumer->GetLastDataset(group_id, 0, "default", &err);
    M_AssertTrue(err == nullptr, "GetLastDataset in group no error");
    consumer->GetLastDataset(group_id, 0, "default", &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetLastDataset in group error second time");

    dataset = consumer->GetNextDataset(group_id, 0, "default", &err);
    M_AssertTrue(err == nullptr, "GetNextDataset2 no error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetNextDataSet2 filename");

    dataset = consumer->GetLastDataset(0, "default", &err);
    M_AssertTrue(err == nullptr, "GetLastDataset2 no error");

    dataset = consumer->GetDatasetById(8, 0, "default", &err);
    M_AssertTrue(err == nullptr, "GetDatasetById error");
    M_AssertTrue(dataset.content[2].name == "8_3", "GetDatasetById filename");

    auto size = consumer->GetCurrentDatasetCount("default", false, &err);
    M_AssertTrue(err == nullptr, "GetCurrentDatasetCount no error");
    M_AssertTrue(size == 10, "GetCurrentDatasetCount size");


// incomplete datasets without min_size

    dataset = consumer->GetNextDataset(group_id, 0, "incomplete", &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetNextDataset incomplete error");
    M_AssertTrue(dataset.content.size() == 2, "GetNextDataset incomplete size");
    M_AssertTrue(dataset.content[0].name == "1_1", "GetNextDataset incomplete filename");
    auto err_data = static_cast<const asapo::PartialErrorData*>(err->GetCustomData());
    M_AssertTrue(err_data->expected_size == 3, "GetDatasetById expected size in error");
    M_AssertTrue(err_data->id == 1, "GetDatasetById expected id in error ");
    M_AssertTrue(dataset.expected_size == 3, "GetDatasetById expected size");
    M_AssertTrue(dataset.id == 1, "GetDatasetById expected id");

    dataset = consumer->GetLastDataset(0, "incomplete", &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kEndOfStream, "GetLastDataset incomplete no data");

    dataset = consumer->GetDatasetById(2, 0, "incomplete", &err);
    M_AssertTrue(err == asapo::ConsumerErrorTemplates::kPartialData, "GetDatasetById incomplete error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete filename");

// incomplete datasets with min_size

    dataset = consumer->GetNextDataset(group_id, 2, "incomplete", &err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 2, "GetDatasetById minsize id");

    dataset = consumer->GetLastDataset(2, "incomplete", &err);
    M_AssertTrue(err == nullptr, "GetNextDataset incomplete minsize error");
    M_AssertTrue(dataset.id == 5, "GetLastDataset minsize id");

    dataset = consumer->GetDatasetById(2, 2, "incomplete", &err);
    M_AssertTrue(err == nullptr, "GetDatasetById incomplete minsize error");
    M_AssertTrue(dataset.content[0].name == "2_1", "GetDatasetById incomplete minsize filename");

    size = consumer->GetCurrentDatasetCount("incomplete", true, &err);
    M_AssertTrue(err == nullptr, "GetCurrentDatasetCount including incomplete no error");
    M_AssertTrue(size == 5, "GetCurrentDatasetCount including incomplete size");

    size = consumer->GetCurrentDatasetCount("incomplete", false, &err);
    M_AssertTrue(err == nullptr, "GetCurrentDatasetCount excluding incomplete no error");
    M_AssertTrue(size == 0, "GetCurrentDatasetCount excluding incomplete size");


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

    consumer->SetTimeout(1000);
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
