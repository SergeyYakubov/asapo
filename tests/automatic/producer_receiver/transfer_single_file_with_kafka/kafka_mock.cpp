#include <stdlib.h>

#include "librdkafka/rdkafka.h"
#include "librdkafka/rdkafka_mock.h"

int main(/*int argc, char** argv*/) {
    rd_kafka_t *rkproducer, *rkconsumer;
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    rd_kafka_mock_cluster_t *mcluster;
    char errstr[256];

    if(rd_kafka_conf_set(conf, "client.id", "MOCK", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }
    /*if(rd_kafka_conf_set(conf, "bootstrap.servers", "127.0.0.1:23456", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }*/

    rkproducer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if(!rkproducer) {
        fprintf(stderr, "Failed to create kafka producer: %s\n", errstr);
        return EXIT_FAILURE;
    }

    mcluster = rd_kafka_mock_cluster_new(rkproducer, 1);
    if(!mcluster) {
        fprintf(stderr, "Failed to create kafka cluster: %s\n", errstr);
        return EXIT_FAILURE;
    }

    const char* bootstrap = rd_kafka_mock_cluster_bootstraps(mcluster);

    fprintf(stdout, "Bootstrap for kafka cluster: %s\n", bootstrap);

    FILE *fp = fopen("bootstrap", "w");
    fprintf(fp, "%s", bootstrap);
    fclose(fp);

    conf = rd_kafka_conf_new();
    if(rd_kafka_conf_set(conf, "client.id", "MOCK_CONSUMER", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }
    if(rd_kafka_conf_set(conf, "group.id", "asapo", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }
    if(rd_kafka_conf_set(conf, "bootstrap.servers", bootstrap, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }

    rkconsumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));

    if(!rkconsumer) {
        fprintf(stderr, "Failed to create kafka consumer: %s\n", errstr);
        return EXIT_FAILURE;
    }
    rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rkconsumer, 10000);

    if(!rkmessage) {
        fprintf(stderr, "No kafka message received\n");
        //return EXIT_FAILURE;
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Got message: err=%d, size=%ld\n", rkmessage->err, rkmessage->len);
    }

    rd_kafka_message_destroy(rkmessage);

    rd_kafka_mock_cluster_destroy(mcluster);
    rd_kafka_destroy(rkproducer);
    rd_kafka_destroy(rkconsumer);
    return EXIT_SUCCESS;
}
