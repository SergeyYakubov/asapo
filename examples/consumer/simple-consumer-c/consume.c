#include "asapo/consumer_c.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
void exit_if_error(const char *error_string, const AsapoError err) {
    if (err) {
      char buf[1024];
        asapo_error_explain(err, buf, sizeof(buf));
      printf("%s %s\n", error_string, buf);
      exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    AsapoError err = NULL;

    const char *endpoint = "enpoint:8400";
    const char *beamtime = "asapo_test";
    const char *token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc=";

    AsapoSourceCredentials cred = asapo_create_source_credentials(kProcessed,
                                                                  beamtime,
                                                                  "", "", token);
    AsapoConsumer consumer = asapo_create_consumer(endpoint,
                                                   "", 1,
                                                   cred,
                                                   &err);
    asapo_delete_source_credentials(&cred);
    
    exit_if_error("Cannot create consumer", err);
    asapo_consumer_set_timeout(consumer, 1000ull);

    AsapoString group_id = asapo_consumer_generate_new_group_id(consumer, &err);
    exit_if_error("Cannot create group id", err);

    AsapoMessageMeta fi = asapo_create_message_meta();
    AsapoMessageData data;

    err = asapo_consumer_get_last(consumer, &fi, &data, group_id);
    exit_if_error("Cannot get next record", err);

    printf("id: %llu\n", asapo_message_meta_get_id(fi));
    printf("file name: %s\n", asapo_message_meta_get_name(fi));
    printf("file content: %s\n", asapo_message_data_get_as_chars(data));
    asapo_delete_message_meta(&fi);
    asapo_delete_message_data(&data);
    asapo_delete_consumer(&consumer);
    asapo_delete_string(&group_id);
    return EXIT_SUCCESS;
}

