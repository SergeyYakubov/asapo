#include "asapo/consumer_c.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
void exit_if_error(const char *error_string, const asapoError err) {
    if (err) {
      char buf[1024];
      asapoErrorExplain(err, buf, sizeof(buf));
      printf("%s %s\n", error_string, buf);
      exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapoError err = NULL;

    const char *endpoint = "asapo-services2:8400";
    const char *beamtime = "asapo_test";
    const char *token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc=";

    asapoSourceCredentials cred = asapoCreateSourceCredentials(kProcessed,
							       beamtime,
							       "", "", token);
    asapoConsumer consumer = asapoCreateConsumer(endpoint,
                                                 "", 1,
                                                 cred,
                                                 &err);
    asapoDeleteSourceCredentials(&cred);
    
    exit_if_error("Cannot create consumer", err);
    asapoConsumerSetTimeout(consumer, 1000ull);

    asapoString group_id = asapoConsumerGenerateNewGroupId(consumer, &err);
    exit_if_error("Cannot create group id", err);

    asapoMessageMeta fi = asapoCreateMessageMeta();
    asapoMessageData data;

    err = asapoConsumerGetLast(consumer,&fi, &data, group_id);
    exit_if_error("Cannot get next record", err);

    printf("id: %llu\n", asapoMessageMetaGetId(fi));
    printf("file name: %s\n", asapoMessageMetaGetName(fi));
    printf("file content: %s\n",asapoMessageDataGetAsChars(data));
    asapoDeleteMessageMeta(&fi);
    asapoDeleteMessageData(&data);
    asapoDeleteConsumer(&consumer);
    asapoDeleteString(&group_id);
    return EXIT_SUCCESS;
}

