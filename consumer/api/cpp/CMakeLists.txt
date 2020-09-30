set(TARGET_NAME asapo-consumer)

set(SOURCE_FILES
        src/data_broker.cpp
        src/server_data_broker.cpp
        src/tcp_client.cpp
        src/tcp_connection_pool.cpp
        src/fabric_consumer_client.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>
            $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:curl_http_client> )

target_include_directories(${TARGET_NAME} PUBLIC include ${ASAPO_CXX_COMMON_INCLUDE_DIR}  ${LIBFABRIC_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
ENDIF()


GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        asapo-fabric ${ASAPO_COMMON_FABRIC_LIBRARIES})


################################
# Testing
################################
set(TEST_SOURCE_FILES
        unittests/test_consumer_api.cpp
        unittests/test_server_broker.cpp
        unittests/test_tcp_client.cpp
        unittests/test_tcp_connection_pool.cpp
        unittests/test_fabric_consumer_client.cpp
        unittests/test_rds_error_mapper.cpp
    )
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

install(TARGETS ${TARGET_NAME} DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)
