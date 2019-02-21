set(TARGET_NAME producer-api)
set(SOURCE_FILES
        src/producer.cpp
        src/producer_impl.cpp
        src/producer_logger.cpp
        src/request_handler_tcp.cpp
        src/request_handler_filesystem.cpp
        src/receiver_discovery_service.cpp
        src/receiver_data_server_request_handler_factory.cpp
        src/producer_request.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:json_parser>
        $<TARGET_OBJECTS:curl_http_client> $<TARGET_OBJECTS:request_pool>)
target_include_directories(${TARGET_NAME} PUBLIC include ${ASAPO_CXX_COMMON_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

################################
# Testing
################################
set(TEST_SOURCE_FILES
        unittests/test_producer_impl.cpp
        unittests/test_producer.cpp
        unittests/test_request_handler_tcp.cpp
        unittests/test_request_handler_filesystem.cpp
        unittests/test_receiver_discovery_service.cpp
        unittests/test_request_handler_factory.cpp
        unittests/test_producer_request.cpp

        )
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

install(TARGETS ${TARGET_NAME} DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)
