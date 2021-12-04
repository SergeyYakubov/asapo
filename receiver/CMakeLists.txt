set(TARGET_NAME receiver)

set(RECEIVER_CORE_FILES
        src/connection.cpp
        src/data_cache.cpp
        src/receiver.cpp
        src/request.cpp
        src/receiver_config.cpp
        src/receiver_logger.cpp
        src/statistics/receiver_statistics.cpp
        src/statistics/statistics.cpp
        src/statistics/statistics_sender_influx_db.cpp
        src/statistics/statistics_sender_fluentd.cpp
        src/request_handler/requests_dispatcher.cpp
        src/request_handler/request_handler_file_process.cpp
        src/request_handler/request_handler_db_write.cpp
        src/request_handler/request_handler_receive_data.cpp
        src/request_handler/request_handler_authorize.cpp
        src/request_handler/request_handler_initial_authorization.cpp
        src/request_handler/request_handler_secondary_authorization.cpp
        src/request_handler/authorization_client.cpp
        src/request_handler/request_handler_db_meta_write.cpp
        src/request_handler/request_handler_db_stream_info.cpp
        src/request_handler/request_handler_db_last_stream.cpp
        src/request_handler/request_handler_receive_metadata.cpp
        src/request_handler/request_handler_db_check_request.cpp
        src/request_handler/request_handler_db_delete_stream.cpp
        src/request_handler/request_handler_db_get_meta.cpp
        src/request_handler/request_handler_kafka_notify.cpp
        src/request_handler/request_factory.cpp
        src/request_handler/request_handler_db.cpp
        src/request_handler/file_processors/write_file_processor.cpp
        src/request_handler/file_processors/file_processor.cpp
        src/request_handler/file_processors/receive_file_processor.cpp
        src/metrics/receiver_prometheus_metrics.cpp
        src/metrics/receiver_mongoose_server.cpp
        )

set(RDS_FILES
        src/receiver_data_server/net_server/fabric_rds_request.cpp
        src/receiver_data_server/receiver_data_server.cpp
        src/receiver_data_server/receiver_data_server_request.cpp
        src/receiver_data_server/receiver_data_server_logger.cpp
        src/receiver_data_server/net_server/rds_tcp_server.cpp
        src/receiver_data_server/net_server/rds_fabric_server.cpp
        src/receiver_data_server/request_handler/receiver_data_server_request_handler_factory.cpp
        src/receiver_data_server/request_handler/receiver_data_server_request_handler.cpp
        )


set(SOURCE_FILES
        ${RECEIVER_CORE_FILES}
        ${RDS_FILES}
        )


################################
# Library
################################
GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:curl_http_client>
        $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:request_pool> $<TARGET_OBJECTS:rdkafka_client>)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC  ${LIBFABRIC_INCLUDE_DIR} ${RDKAFKA_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} CURL::libcurl ${RDKAFKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} database
        asapo-fabric ${ASAPO_COMMON_FABRIC_LIBRARIES})


add_executable(${TARGET_NAME}-bin src/main.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME ${TARGET_NAME})
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

set_target_properties(${TARGET_NAME}-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

if (WIN32)
    file(GLOB KAFKA_DLLS "${RDKAFKA_BIN_DIR}/rdkafka*.dll")
    file(COPY ${KAFKA_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

configure_file(docker/Dockerfile . COPYONLY)
configure_file(docker/install_libfabric.sh . COPYONLY)


################################
# Testing
################################

set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)
#
set(TEST_SOURCE_FILES
        unittests/test_receiver.cpp
        unittests/test_connection.cpp
        unittests/statistics/test_statistics.cpp
        unittests/statistics/test_receiver_statistics.cpp
        unittests/test_config.cpp
        unittests/test_request.cpp
        unittests/request_handler/test_request_factory.cpp
        unittests/request_handler/test_request_handler_file_process.cpp
        unittests/request_handler/test_request_handler_db_writer.cpp
        unittests/request_handler/test_request_handler_db_check_request.cpp
        unittests/request_handler/test_request_handler_db_meta_writer.cpp
        unittests/request_handler/test_request_handler_db_stream_info.cpp
        unittests/request_handler/test_request_handler_db_last_stream.cpp
        unittests/request_handler/test_request_handler_db.cpp
        unittests/request_handler/test_authorization_client.cpp
        unittests/request_handler/test_request_handler_initial_authorization.cpp
        unittests/request_handler/test_request_handler_secondary_authorization.cpp
        unittests/request_handler/test_request_handler_receive_data.cpp
        unittests/request_handler/test_request_handler_receive_metadata.cpp
        unittests/request_handler/test_request_handler_delete_stream.cpp
        unittests/request_handler/test_request_handler_db_get_meta.cpp
        unittests/request_handler/test_request_handler_kafka_notify.cpp
        unittests/statistics/test_statistics_sender_influx_db.cpp
        unittests/statistics/test_statistics_sender_fluentd.cpp
        unittests/mock_receiver_config.cpp
        unittests/request_handler/test_requests_dispatcher.cpp
        unittests/test_datacache.cpp
        unittests/request_handler/file_processors/test_write_file_processor.cpp
        unittests/request_handler/file_processors/test_file_processor.cpp
        unittests/request_handler/file_processors/test_receive_file_processor.cpp
        )
#
set(TEST_LIBRARIES "${TARGET_NAME};system_io")
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}"
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/net_server/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/net_server/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/request_handler/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver_data_server/request_handler/*.h
        )

set(TEST_SOURCE_FILES_RDS
        unittests/receiver_data_server/test_receiver_data_server.cpp
        unittests/receiver_data_server/net_server/test_rds_fabric_server.cpp
        unittests/receiver_data_server/net_server/test_rds_tcp_server.cpp
        unittests/receiver_data_server/request_handler/test_request_handler_factory.cpp
        unittests/receiver_data_server/request_handler/test_request_handler.cpp
        )
gtest(${TARGET_NAME}_RDS "${TEST_SOURCE_FILES_RDS}" "${TEST_LIBRARIES}" nocov)
