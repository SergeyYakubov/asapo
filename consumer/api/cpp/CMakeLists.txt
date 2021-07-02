set(TARGET_NAME asapo-consumer)

set(SOURCE_FILES
        src/consumer.cpp
        src/consumer_impl.cpp
        src/tcp_consumer_client.cpp
        src/tcp_connection_pool.cpp
        src/fabric_consumer_client.cpp
        src/consumer_c_glue.cpp)


################################
# Libraries
################################

GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)

add_library(consumer_lib_objects OBJECT ${SOURCE_FILES})
target_include_directories(consumer_lib_objects PRIVATE ../c/include include ${ASAPO_CXX_COMMON_INCLUDE_DIR} )
target_include_directories(consumer_lib_objects SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})


if (BUILD_STATIC_CLIENT_LIBS)
    add_library(${TARGET_NAME} STATIC $<TARGET_OBJECTS:consumer_lib_objects> $<TARGET_OBJECTS:asapo_fabric_objects>  $<TARGET_OBJECTS:system_io>
            $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>  $<TARGET_OBJECTS:curl_http_client> )
    target_include_directories(${TARGET_NAME} PUBLIC ../c/include include ${ASAPO_CXX_COMMON_INCLUDE_DIR})
    target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ASAPO_COMMON_FABRIC_LIBRARIES})
endif()

if (BUILD_SHARED_CLIENT_LIBS)
    add_library(${TARGET_NAME}_shared SHARED $<TARGET_OBJECTS:consumer_lib_objects>  $<TARGET_OBJECTS:asapo_fabric_objects> $<TARGET_OBJECTS:system_io>
            $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>  $<TARGET_OBJECTS:curl_http_client> )
    target_include_directories(${TARGET_NAME}_shared PUBLIC ../c/include include ${ASAPO_CXX_COMMON_INCLUDE_DIR})
    target_include_directories(${TARGET_NAME}_shared SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME}_shared ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ASAPO_COMMON_FABRIC_LIBRARIES})
endif()





################################
# Testing
################################
set(TEST_SOURCE_FILES
        unittests/test_consumer_api.cpp
        unittests/test_consumer_impl.cpp
        unittests/test_tcp_consumer_client.cpp
        unittests/test_tcp_connection_pool.cpp
        unittests/test_fabric_consumer_client.cpp
        unittests/test_rds_error_mapper.cpp
    )
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer_c_glue.cpp)

install(TARGETS ${TARGET_NAME} DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
