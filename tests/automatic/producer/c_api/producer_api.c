#include "asapo/producer_c.h"
#include "testing_c.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
void TestMeta(const std::unique_ptr<asapo::Producer>& producer) {
    asapo::Error err;
    std::string meta = R"({"data":"test","embedded":{"edata":2}})";
    producer->SendBeamtimeMetadata(meta, asapo::MetaIngestMode{asapo::MetaIngestOp::kInsert, true}, nullptr);
    producer->WaitRequestsFinished(5000);
    auto meta_received = producer->GetBeamtimeMeta(5000, &err);
    M_AssertTrue(meta_received == meta);
    std::string meta_update = R"({"embedded":{"edata":3}})";
    std::string meta_updated = R"({"data":"test","embedded":{"edata":3}})";
    producer->SendBeamtimeMetadata(meta_update, asapo::MetaIngestMode{asapo::MetaIngestOp::kUpdate, false}, nullptr);
    producer->WaitRequestsFinished(5000);
    meta_received = producer->GetBeamtimeMeta(5000, &err);
    M_AssertTrue(meta_received == meta_updated);
}


void Test(const std::unique_ptr<asapo::Producer>& producer) {
    asapo::MessageMeta fi;
    asapo::Error err;

    std::string client, server;
    bool supported;
    err = producer->GetVersionInfo(&client, &server, &supported);
    M_AssertTrue(err == nullptr, "Version OK");
    M_AssertTrue(supported, "client supported by server");
    M_AssertTrue(!client.empty(), "client version");
    M_AssertTrue(!server.empty(), "server version");


    TestMeta(producer);

    producer->GetStreamInfo("default", 5000, &err);
    if (err) {
        printf("%s\n", err->Explain().c_str());
    }
    M_AssertTrue(err == nullptr, "stream info");

}


std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, 2,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed, args.beamtime,
                                                    "", args.source, ""}, 60000, &err);
    if (err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Debug);
    return producer;
}

 */

void callback(AsapoRequestCallbackPayloadHandle payload, AsapoErrorHandle error) {
    EXIT_IF_ERROR("error after callback", error);
    AsapoMessageDataHandle data_handle = asapo_request_callback_payload_get_data(payload);
    AsapoStringHandle response = asapo_request_callback_payload_get_response(payload);
    const struct AsapoGenericRequestHeader* header = asapo_request_callback_payload_get_original_header(payload);

    printf("%d\n",(int)header->data_id);

    asapo_free_handle(&data_handle);
    asapo_free_handle(&response);
}

void test_send(AsapoProducerHandle producer) {
    AsapoErrorHandle err = asapo_new_handle();
    AsapoMessageHeaderHandle message_header = NULL;

    char data[] = "hello";
    asapo_producer_send(producer,
                            message_header,
                            data,
                            kDefaultIngestMode,
                            "default",
                            callback,
                            &err);

}

int main(int argc, char* argv[]) {
    if (argc <4) {
        abort();
    }
    const char *endpoint = argv[1];
    const char *source = argv[2];
    const char *beamtime = argv[3];


    AsapoErrorHandle err = asapo_new_handle();
    AsapoSourceCredentialsHandle cred = asapo_create_source_credentials(kProcessed,
                                                                        beamtime,
                                                                        "", source, "");

    AsapoProducerHandle producer = asapo_create_producer(endpoint,2,kTcp, cred,60000,&err);
    EXIT_IF_ERROR("create producer", err);

    asapo_producer_enable_local_log(producer, 1);
    asapo_producer_set_log_level(producer, Debug);

    test_send(producer);

    asapo_free_handle(&err);
    asapo_free_handle(&cred);
    asapo_free_handle(&producer);
    return 0;
}
