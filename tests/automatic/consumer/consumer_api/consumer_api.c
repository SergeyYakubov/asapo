#include "asapo/consumer_c.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void assert_eq_int(int expected, int got, const char *message) {
    if (expected!=got) {
        printf("%s: expected %d got %d at %d\n",message, expected, got,__LINE__);
        exit(EXIT_FAILURE);
    }
}

void assert_eq_string(const char * expected, const char *got, const char *message) {
    if (strcmp(expected,got)!=0) {
        printf("%s: expected %s got %s at %d\n",message, expected, got,__LINE__);
        exit(EXIT_FAILURE);
    }
}

void assert_bool(int value, const char *message) {
    if (value!=1) {
        printf("%s failed at %d\n",message, __LINE__);
        exit(EXIT_FAILURE);
    }
}


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

    const char *endpoint = argv[1];
    const char *beamtime = argv[2];
    const char *token = argv[3];

    if (strcmp(argv[4],"dataset") == 0) { // do not test datasets for now
        exit(0);
    }

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

    printf("group id: %s\n",asapo_string_c_str(group_id));

    AsapoMessageMeta fi;
    AsapoMessageData data;

    err = asapo_consumer_get_last(consumer, &fi, NULL, "default");
    exit_if_error("Cannot get last record", err);

    assert_eq_int(10,asapo_message_meta_get_id(fi),"id");
    assert_eq_string("10",asapo_message_meta_get_name(fi),"id");

    asapo_delete_message_meta(&fi);
    asapo_delete_message_data(&data);
    asapo_delete_consumer(&consumer);
    asapo_delete_string(&group_id);
    return EXIT_SUCCESS;
}

