set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/receiver.cpp
        src/connection.cpp
        src/request.cpp
        src/request_handler_file_write.cpp
        src/statistics.cpp
        src/statistics_sender_influx_db.cpp
        src/receiver_config.cpp
        src/receiver_logger.cpp
        src/request_handler_db_write.cpp
        src/request_handler_receive_data.cpp
        src/request_handler_authorize.cpp
        src/statistics_sender_fluentd.cpp
        src/requests_dispatcher.cpp
        src/receiver_data_server/receiver_data_server.cpp
        src/receiver_data_server/tcp_server.cpp
        src/receiver_data_server/receiver_data_server_request.cpp
        src/receiver_data_server/receiver_data_server_logger.cpp
        src/data_cache.cpp
        src/receiver_data_server/receiver_data_server_request_handler_factory.cpp
        src/receiver_data_server/receiver_data_server_request_handler.cpp
        src/receiver_statistics.cpp
        src/request_handler_db_meta_write.cpp
        src/request_handler_receive_metadata.cpp
        src/request_handler_file_receive.cpp
        src/request_factory.cpp
        src/request_handler_db.cpp)


################################
# Library
################################
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static")


add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:curl_http_client>
         $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:request_pool>)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} database)


add_executable(${TARGET_NAME}-bin src/main.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME ${TARGET_NAME})
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

set_target_properties(${TARGET_NAME}-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

configure_file(docker/Dockerfile . COPYONLY)


################################
# Testing
################################

set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)
#
set(TEST_SOURCE_FILES
        unittests/test_receiver.cpp
        unittests/test_connection.cpp
        unittests/test_statistics.cpp
        unittests/test_receiver_statistics.cpp
        unittests/test_config.cpp
        unittests/test_request.cpp
        unittests/test_request_factory.cpp
        unittests/test_request_handler_file_write.cpp
        unittests/test_request_handler_file_receive.cpp
        unittests/test_request_handler_db_writer.cpp
        unittests/test_request_handler_db_meta_writer.cpp
        unittests/test_request_handler_db.cpp
        unittests/test_request_handler_authorizer.cpp
        unittests/test_request_handler_receive_data.cpp
        unittests/test_request_handler_receive_metadata.cpp
        unittests/test_statistics_sender_influx_db.cpp
        unittests/test_statistics_sender_fluentd.cpp
        unittests/mock_receiver_config.cpp
        unittests/test_requests_dispatcher.cpp
        unittests/test_datacache.cpp
        )
#
set(TEST_LIBRARIES "${TARGET_NAME};system_io")
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

set(TEST_SOURCE_FILES_RDS
        unittests/receiver_data_server/test_receiver_data_server.cpp
        unittests/receiver_data_server/test_tcp_server.cpp
        unittests/receiver_data_server/test_request_handler_factory.cpp
        unittests/receiver_data_server/test_request_handler.cpp
        )
gtest(${TARGET_NAME}_RDS "${TEST_SOURCE_FILES_RDS}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
