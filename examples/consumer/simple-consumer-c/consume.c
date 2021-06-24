#include "asapo/consumer_c.h"

#include <stdio.h>
#include <stdlib.h>

void exit_if_error(const char *error_string, const AsapoErrorHandle err) {
    if (err) {
        char buf[1024];
        asapo_error_explain(err, buf, sizeof(buf));
        printf("%s %s\n", error_string, buf);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    AsapoErrorHandle err = asapo_new_handle();
    AsapoMessageMetaHandle mm = asapo_new_handle();
    AsapoMessageDataHandle data = asapo_new_handle();

    const char *endpoint = "localhost:8400";
    const char *beamtime = "asapo_test";
    const char *token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMWY2OG0ydWlkZDE3dWxmaDN1ZyIsInN1YiI6ImJ0X2FzYXBvX3Rlc3QiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlcyI6WyJyZWFkIl19fQ.zo7ZDfY2sf4o9RYuXpxNR9kHLG594xr-SE5yLoyDC2Q";

    const char * path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"; //set it according to your configuration.

    AsapoSourceCredentialsHandle cred = asapo_create_source_credentials(kProcessed,
                                                                        beamtime,
                                                                        "", "", token);
    AsapoConsumerHandle consumer = asapo_create_consumer(endpoint,
                                                         path_to_files, 1,
                                                         cred,
                                                         &err);
    asapo_free_handle(&cred);

    exit_if_error("Cannot create consumer", err);
    asapo_consumer_set_timeout(consumer, 1000ull);

    AsapoStringHandle group_id = asapo_consumer_generate_new_group_id(consumer, &err);
    exit_if_error("Cannot create group id", err);

    asapo_consumer_get_next(consumer, group_id, &mm, &data, "default",&err);
    exit_if_error("Cannot get next record", err);

    printf("id: %llu\n", asapo_message_meta_get_id(mm));
    printf("file name: %s\n", asapo_message_meta_get_name(mm));
    printf("file content: %s\n", asapo_message_data_get_as_chars(data));

    asapo_free_handle(&mm);
    asapo_free_handle(&data);
    asapo_free_handle(&consumer);
    asapo_free_handle(&group_id);

    return EXIT_SUCCESS;
}