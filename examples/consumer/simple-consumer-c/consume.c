#include <string.h>
#include "consumer_c.h"

void exit_if_error(const char *error_string, const asapoError err) {
    if (err) {
      char buf[1024];
      asapoErrorExplain(err, buf, sizeof(buf));
      printf("%s %s\n", error_string, buf);
      exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapoError err;

    const char *endpoint = "asapo-services2:8400";
    const char *beamtime = "asapo_test";
    const char *token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc=";

    asapoSourceCredentials cred = asapoCreateSourceCredentials("processed",
							       beamtime,
							       "", "", token);
    asapoConsumer consumer = asapoCreateConsumer(endpoint,
						 "", true,
						 asapo::SourceCredentials{beamtime,
						   "", "", token},
						 &err);
    asapoDeleteSourceCredentials(&cred);
    
    exit_if_error("Cannot create consumer", err);
    consumer->SetTimeout((uint64_t) 1000);

    asapoGroupId group_id = asapoConsumerGenerateNewGroupId(consumer, &err);
    exit_if_error("Cannot create group id", err);

    asapoMessageMeta fi = asapoCreateMessageMeta();
    asapoMessageData data;

    err = asappConsumerGetLast(consumer,&fi, group_id, &data);
    exit_if_error("Cannot get next record", err);

    printf("id: %llu\n", asapoMessageMetaGetId(fi);
    printf("file name: %s\n", asapoMessageMetaGetName(fi);
    std::cout << "file content: " << reinterpret_cast<char const*>(data.get()) << std::endl;
    asapoDeleteMessageMeta(&fi);
    asapoDeleteConsumer(&consumer);      
    return EXIT_SUCCESS;
}

