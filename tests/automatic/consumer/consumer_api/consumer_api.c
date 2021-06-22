#include "asapo/consumer_c.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define EXIT_IF_ERROR(...) exit_if_error_(__VA_ARGS__,__LINE__)
#define ASSERT_EQ_INT(...) assert_eq_int_(__VA_ARGS__,__LINE__)
#define ASSERT_EQ_STRING(...) assert_eq_string_(__VA_ARGS__,__LINE__)
#define ASSERT_TRUE(...) assert_true_(__VA_ARGS__,__LINE__)

void assert_eq_int_(int expected, int got, const char *message, int line) {
    printf("asserting %s at %d\n",message,line);
    if (expected!=got) {
        printf("%s: expected %d got %d at %d\n",message, expected, got,line);
        exit(EXIT_FAILURE);
    }
}

void assert_eq_string_(const char * expected, const char *got, const char *message, int line) {
    printf("asserting %s at %d\n",message,line);
    if (strcmp(expected,got)!=0) {
        printf("%s: expected %s got %s at %d\n",message, expected, got,line);
        exit(EXIT_FAILURE);
    }
}

void assert_true_(int value, const char *message, int line) {
    printf("asserting %s at %d\n",message,line);
    if (value!=1) {
        printf("%s failed at %d\n",message, line);
        exit(EXIT_FAILURE);
    }
}

void exit_if_error_(const char *error_string, const AsapoErrorHandle err, int line) {
    printf("asserting no error for %s at %d\n",error_string,line);
    if (asapo_is_error(err)) {
        char buf[1024];
        asapo_error_explain(err, buf, sizeof(buf));
        printf("%s %s\n", error_string, buf);
        exit(EXIT_FAILURE);
    }
}


void test_single(AsapoConsumerHandle consumer, AsapoStringHandle group_id) {
    AsapoErrorHandle err = NULL;
    AsapoMessageMetaHandle md = NULL;
    AsapoMessageDataHandle data = NULL;

//next
    err = asapo_consumer_get_next(consumer, group_id, &md, NULL, "default");
    EXIT_IF_ERROR("asapo_consumer_get_next", err);
    ASSERT_TRUE(!strcmp(asapo_message_meta_get_name(md),"1"), "get next filename");
    ASSERT_TRUE(!strcmp(asapo_message_meta_get_metadata(md),"{\"test\":10}"), "get next  metadata");
    asapo_free_handle(&err);

// retrieve
    err = asapo_consumer_retrieve_data(consumer,md, &data);
    EXIT_IF_ERROR("asapo_consumer_retrieve_data", err);
    ASSERT_EQ_INT(6,asapo_message_meta_get_size(md),"asapo_message_meta_get_size");
    ASSERT_TRUE(strncmp("hello1", asapo_message_data_get_as_chars(data), asapo_message_meta_get_size(md))==0, "get next  metadata");
    asapo_free_handle(&err);
    asapo_free_handle(&data);

//last
    err = asapo_consumer_get_last(consumer, &md, NULL, "default");
    EXIT_IF_ERROR("asapo_consumer_get_last", err);
    ASSERT_EQ_INT(10,asapo_message_meta_get_id(md),"id");
    ASSERT_EQ_STRING("10",asapo_message_meta_get_name(md),"id");
    asapo_free_handle(&err);

//id
    err = asapo_consumer_get_by_id(consumer,8, &md, NULL, "default");
    EXIT_IF_ERROR("asapo_consumer_get_by_id", err);
    ASSERT_EQ_STRING("8",asapo_message_meta_get_name(md),"id");
    asapo_free_handle(&err);

// last read marker
    err = asapo_consumer_set_last_read_marker(consumer, group_id, 2, "default");
    EXIT_IF_ERROR("asapo_consumer_set_last_read_marker", err);
    asapo_free_handle(&err);
    err = asapo_consumer_get_next(consumer, group_id, &md, NULL, "default");
    EXIT_IF_ERROR("asapo_consumer_get_next", err);
    ASSERT_TRUE(!strcmp(asapo_message_meta_get_name(md),"3"), "get next asapo_consumer_set_last_read_marker ");
    asapo_free_handle(&err);

    err = asapo_consumer_reset_last_read_marker(consumer, group_id, "default");
    EXIT_IF_ERROR("asapo_consumer_reset_last_read_marker", err);
    asapo_free_handle(&err);
    err = asapo_consumer_get_next(consumer, group_id, &md, NULL, "default");
    ASSERT_TRUE(!strcmp(asapo_message_meta_get_name(md),"1"), "get next asapo_consumer_reset_last_read_marker ");
    asapo_free_handle(&err);

// stream size
    uint64_t size = asapo_consumer_get_current_size(consumer,"default", &err);
    EXIT_IF_ERROR("asapo_consumer_get_current_size", err);
    ASSERT_EQ_INT(10,size,"asapo_consumer_get_current_size");
    asapo_free_handle(&err);

    size = asapo_consumer_get_current_size(consumer,"stream1", &err);
    EXIT_IF_ERROR("asapo_consumer_get_current_size stream1", err);
    ASSERT_EQ_INT(5,size,"asapo_consumer_get_current_size stream1");
    asapo_free_handle(&err);

// query messages
    AsapoMessageMetasHandle messages = asapo_consumer_query_messages(consumer, "meta.test = 10 AND name='1'", "default", &err);
    EXIT_IF_ERROR("asapo_consumer_query_messages", err);
    ASSERT_EQ_INT(1,asapo_message_metas_get_size(messages),"asapo_consumer_query_messages");
    asapo_free_handle(&err);

// stream list
    AsapoStreamInfosHandle streams = asapo_consumer_get_stream_list(consumer, "", kAllStreams , &err);
    EXIT_IF_ERROR("asapo_consumer_get_stream_list", err);
    ASSERT_EQ_INT(3,asapo_stream_infos_get_size(streams),"asapo_consumer_get_stream_list size");

    AsapoStreamInfoHandle s0 = asapo_stream_infos_get_item(streams,0);
    AsapoStreamInfoHandle s1 = asapo_stream_infos_get_item(streams,1);
    AsapoStreamInfoHandle s2 = asapo_stream_infos_get_item(streams,2);

    ASSERT_EQ_STRING("default",asapo_stream_info_get_name(s0),"streams0.name");
    ASSERT_EQ_STRING("stream1",asapo_stream_info_get_name(s1),"streams1.name");
    ASSERT_EQ_STRING("stream2",asapo_stream_info_get_name(s2),"streams2.name");
    ASSERT_TRUE(asapo_stream_info_get_ffinished(s1),"streams1 finished");
    ASSERT_EQ_STRING("ns",asapo_stream_info_get_next_stream(s1),"stream1 next stream");
    asapo_free_handle(&err);

    struct timespec time;
    asapo_stream_info_get_timestamp_last_entry(s1,&time);
    ASSERT_EQ_INT(0,time.tv_sec,"stream 1 lastentry sec");
    ASSERT_EQ_INT(1000,time.tv_nsec,"stream 1 lastentry nsec");

    asapo_stream_info_get_timestamp_created(s2,&time);
    ASSERT_EQ_INT(0,time.tv_sec,"stream 2 timestamp_created sec");
    ASSERT_EQ_INT(2000,time.tv_nsec,"stream 2 timestamp_created nsec");
    asapo_free_handle(&s0);
    asapo_free_handle(&s1);
    asapo_free_handle(&s2);

// acknowledges
    uint64_t id = asapo_consumer_get_last_acknowledged_message(consumer,group_id, "default", &err);
    ASSERT_TRUE(asapo_error_get_type(err) == kNoData,"last ack default stream no data");
    ASSERT_EQ_INT(0,id,"last ack default stream no data id = 0");
    asapo_free_handle(&err);

    AsapoIdListHandle nacks = asapo_consumer_get_unacknowledged_messages(consumer, group_id, 0, 0, "default", &err);
    EXIT_IF_ERROR("asapo_consumer_get_unacknowledged_messages", err);
    ASSERT_EQ_INT(10,asapo_id_list_get_size(nacks),"last ack default stream no data id = 0");
    asapo_free_handle(&err);

    err = asapo_consumer_acknowledge(consumer,group_id, 1, "default");
    EXIT_IF_ERROR("asapo_consumer_acknowledge", err);
    asapo_free_handle(&err);

    err = asapo_consumer_negative_acknowledge(consumer,group_id, 1, 0, "default");
    EXIT_IF_ERROR("asapo_consumer_negative_acknowledge", err);
    asapo_free_handle(&err);

    asapo_consumer_set_resend_nacs(consumer,1, 0, 1);

// stream deletion

    err = asapo_consumer_delete_stream(consumer,"default", 1,1);
    EXIT_IF_ERROR("asapo_consumer_delete_stream", err);
    asapo_free_handle(&err);

    err = asapo_consumer_delete_stream(consumer,"default", 1,1);
    ASSERT_TRUE(asapo_error_get_type(err) == kWrongInput,"delete non existing stream");
    asapo_free_handle(&err);

    asapo_free_handle(&messages);
    asapo_free_handle(&streams);
    asapo_free_handle(&nacks);
    asapo_free_handle(&md);
}

int main(int argc, char* argv[]) {
    const char *endpoint = argv[1];
    const char *beamtime = argv[2];
    const char *token = argv[3];

    AsapoErrorHandle err = asapo_init_handle();
    AsapoSourceCredentialsHandle cred = asapo_create_source_credentials(kProcessed,
                                                                        beamtime,
                                                                        "", "", token);
    AsapoConsumerHandle consumer = asapo_create_consumer(endpoint,
                                                         ".", 1,
                                                         cred,
                                                         &err);
    EXIT_IF_ERROR("create consumer", err);
    asapo_free_handle(&err);

    asapo_consumer_set_timeout(consumer, 1000ull);


    AsapoStringHandle group_id = asapo_consumer_generate_new_group_id(consumer, &err);
    EXIT_IF_ERROR("create group id", err);
    asapo_free_handle(&err);

    if (strcmp(argv[4],"single") == 0) {
        test_single(consumer,group_id);
    }

    if (strcmp(argv[4],"dataset") == 0) {
//        exit(0);
    }

    asapo_free_handle(&cred);
    asapo_free_handle(&consumer);
    asapo_free_handle(&group_id);

    return EXIT_SUCCESS;
}

