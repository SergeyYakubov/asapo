#include "asapo/consumer_c.h"

#include <string.h>
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
    AsapoErrorHandle err = NULL;

    const char *endpoint = "enpoint:8400";
    const char *beamtime = "asapo_test";
    const char *token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc=";

    AsapoSourceCredentialsHandle cred = asapo_create_source_credentials(kProcessed,
                                                                        beamtime,
                                                                        "", "", token);
    AsapoConsumerHandle consumer = asapo_create_consumer(endpoint,
                                                         "", 1,
                                                         cred,
                                                         &err);
    asapo_free_handle(&cred);
    
    exit_if_error("Cannot create consumer", err);
    asapo_consumer_set_timeout(consumer, 1000ull);

    AsapoStringHandle group_id = asapo_consumer_generate_new_group_id(consumer, &err);
    exit_if_error("Cannot create group id", err);

    AsapoMessageMetaHandle fi;
    AsapoMessageDataHandle data;
    asapo_consumer_get_last(consumer, &fi, &data, group_id,&err);
    exit_if_error("Cannot get next record", err);

    printf("id: %llu\n", asapo_message_meta_get_id(fi));
    printf("file name: %s\n", asapo_message_meta_get_name(fi));
    printf("file content: %s\n", asapo_message_data_get_as_chars(data));
    asapo_free_handle(&fi);
    asapo_free_handle(&data);
    asapo_free_handle(&consumer);
    asapo_free_handle(&group_id);
    return EXIT_SUCCESS;
}

