#include "asapo/producer_c.h"
#include "testing_c.h"

#include <string.h>
#include <stdlib.h>

void callback(void* original_data, AsapoRequestCallbackPayloadHandle payload, AsapoErrorHandle error) {
    EXIT_IF_ERROR("error after callback", error);
    AsapoStringHandle response = asapo_request_callback_payload_get_response(payload);
    const struct AsapoGenericRequestHeader* header = asapo_request_callback_payload_get_original_header(payload);

    ASSERT_EQ_INT(1,header->data_id,"data id");
    ASSERT_EQ_STRING("hello",(const char*)original_data,"data in payload");

    free(original_data);

    asapo_free_handle(&response);
}

void test_send(AsapoProducerHandle producer) {
    AsapoErrorHandle err = asapo_new_handle();

    AsapoMessageHeaderHandle message_header = asapo_create_message_header(1,6,
        "processed/test","",0,0,0);
    char* data = ( char* )malloc( 6 *sizeof( char ) );
    strcpy(data,"hello");

    asapo_producer_send(producer,
                            message_header,
                            data,
                            kDefaultIngestMode,
                            "default",
                            callback,
                            &err);
    EXIT_IF_ERROR("error sending data", err);

    asapo_producer_send_stream_finished_flag(producer,"default",1,"",NULL,&err);
    EXIT_IF_ERROR("asapo_producer_send_stream_finished_flag", err);

    asapo_producer_wait_requests_finished(producer,2000,&err);
    EXIT_IF_ERROR("asapo_producer_wait_requests_finished", err);

    asapo_free_handle(&err);
    asapo_free_handle(&message_header);
}

void test_meta(AsapoProducerHandle producer) {
    AsapoErrorHandle err = asapo_new_handle();
    char meta[] = "{\"data\":\"test\",\"embedded\":{\"edata\":2}}";
    asapo_producer_send_beamtime_metadata(producer,meta,kInsert,1,NULL,&err);
    asapo_producer_wait_requests_finished(producer,5000,NULL);
    AsapoStringHandle meta_received = asapo_producer_get_beamtime_meta(producer,5000, &err);
    EXIT_IF_ERROR("asapo_producer_get_beamtime_meta", err);
    ASSERT_EQ_STRING(meta,asapo_string_c_str(meta_received),"returned same meta as was ingested");

    asapo_producer_send_stream_metadata(producer,meta,kInsert,1,"default", NULL,&err);
    asapo_producer_wait_requests_finished(producer,5000,NULL);
    AsapoStringHandle stream_meta_received = asapo_producer_get_stream_meta(producer,"default",5000, &err);
    EXIT_IF_ERROR("asapo_producer_send_stream_metadata", err);
    ASSERT_EQ_STRING(meta,asapo_string_c_str(stream_meta_received),"stream meta returned same meta as was ingested");
    asapo_free_handle(&err);
    asapo_free_handle(&meta_received);
    asapo_free_handle(&stream_meta_received);
}

void test_streams(AsapoProducerHandle producer) {
    AsapoErrorHandle err = asapo_new_handle();
    AsapoStreamInfoHandle sinfo = asapo_producer_get_stream_info(producer,"default",2000,&err);
    EXIT_IF_ERROR("asapo_producer_get_stream_info", err);
    ASSERT_EQ_STRING("default",asapo_stream_info_get_name(sinfo),"stream name");

    AsapoStreamInfoHandle sinfo_last = asapo_producer_get_last_stream(producer,2000,&err);
    EXIT_IF_ERROR("asapo_producer_get_last_stream", err);
    ASSERT_EQ_STRING("default",asapo_stream_info_get_name(sinfo_last),"stream name");
    ASSERT_EQ_INT(1,(uint64_t)asapo_stream_info_get_ffinished(sinfo_last),"stream finished");
    ASSERT_EQ_INT(2,asapo_stream_info_get_last_id(sinfo_last),"last id 0");

    asapo_free_handle(&sinfo_last);

    asapo_producer_delete_stream(producer,"default",5000,1,1,&err);
    EXIT_IF_ERROR("asapo_producer_delete_stream", err);

    sinfo_last = asapo_producer_get_last_stream(producer,2000,&err);
    EXIT_IF_ERROR("asapo_producer_get_last_stream after deletion", err);
    ASSERT_EQ_INT(0,asapo_stream_info_get_last_id(sinfo_last),"last id 0");


    asapo_free_handle(&err);
    asapo_free_handle(&sinfo);
    asapo_free_handle(&sinfo_last);
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
                                                                        "auto", "auto",
                                                                        beamtime, "", source, "");

    AsapoProducerHandle producer = asapo_create_producer(endpoint,3,kTcp, cred,60000,&err);
    EXIT_IF_ERROR("create producer", err);

    asapo_producer_enable_local_log(producer, 1);
    asapo_producer_set_log_level(producer, Debug);

    test_send(producer);
    test_meta(producer);
    test_streams(producer);

    asapo_free_handle(&err);
    asapo_free_handle(&cred);
    asapo_free_handle(&producer);
    return EXIT_SUCCESS;
}
