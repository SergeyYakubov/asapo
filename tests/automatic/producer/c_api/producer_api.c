#include "asapo/producer_c.h"
#include "testing_c.h"

#include <string.h>
#include <stdio.h>
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


    asapo_producer_wait_requests_finished(producer,2000,&err);
    EXIT_IF_ERROR("asapo_producer_wait_requests_finished", err);

    asapo_free_handle(&err);
    asapo_free_handle(&message_header);
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

    AsapoProducerHandle producer = asapo_create_producer(endpoint,3,kTcp, cred,60000,&err);
    EXIT_IF_ERROR("create producer", err);

    asapo_producer_enable_local_log(producer, 1);
    asapo_producer_set_log_level(producer, Debug);

    test_send(producer);

    asapo_free_handle(&err);
    asapo_free_handle(&cred);
    asapo_free_handle(&producer);
    return 0;
}
