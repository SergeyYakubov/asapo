#include <stdlib.h>
#include <string.h>

#include "librdkafka/rdkafka.h"
#include "librdkafka/rdkafka_mock.h"

int main(int argc, char *argv[]) {

    char *expectedmsg;
    asprintf(&expectedmsg, "{\"event\":\"IN_CLOSE_WRITE\",\"path\":\"%s\"}", argc > 1 ? argv[argc - 1] : "processed/1");

    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    char errstr[256];

    if(rd_kafka_conf_set(conf, "client.id", "MOCK", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }
    /*if(rd_kafka_conf_set(conf, "bootstrap.servers", "127.0.0.1:23456", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "Failed to set kafka config: %s\n", errstr);
        return EXIT_FAILURE;
    }*/

    rd_kafka_t *rkproducer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if(!rkproducer) {
        fprintf(stderr, "Failed to create kafka producer: %s\n", errstr);
        return EXIT_FAILURE;
    }

    rd_kafka_mock_cluster_t *mcluster = rd_kafka_mock_cluster_new(rkproducer, 1);
    if(!mcluster) {
        fprintf(stderr, "Failed to create kafka cluster: %s\n", errstr);
        return EXIT_FAILURE;
    }

    const char* bootstrap = rd_kafka_mock_cluster_bootstraps(mcluster);

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

    rd_kafka_t *rkconsumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));

    if(!rkconsumer) {
        fprintf(stderr, "Failed to create kafka consumer: %s\n", errstr);
        return EXIT_FAILURE;
    }

    rd_kafka_poll_set_consumer(rkconsumer);
    rd_kafka_topic_partition_list_t* subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, "asapo", RD_KAFKA_PARTITION_UA);
    rd_kafka_resp_err_t err = rd_kafka_subscribe(rkconsumer, subscription);
    if (err) {
        fprintf(stderr, "Failed to subscribe to topic: %s\n", rd_kafka_err2str(err));
        return EXIT_FAILURE;
    }


    rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rkconsumer, 30000);

    if(!rkmessage) {
        fprintf(stderr, "No kafka message received\n");
        return EXIT_FAILURE;
    } else {
        if (rkmessage->err) {
            fprintf(stderr, "Got error: %s\n", rd_kafka_message_errstr(rkmessage));
            return EXIT_FAILURE;
        } else {
            if (!strncmp((const char *)rkmessage->payload, expectedmsg, rkmessage->len)) {
                fprintf(stdout, "Kafka message is correct: %.*s\n", (int)rkmessage->len, (const char *)rkmessage->payload);
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, "Kafka message is incorrect: %.*s (expected %s)\n", (int)rkmessage->len, (const char *)rkmessage->payload, expectedmsg);
                return EXIT_FAILURE;
            }
        }
    }
}
