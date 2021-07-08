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

string(REPLACE "v" "" CONSUMER_CLIENT_VER ${ASAPO_CONSUMER_PROTOCOL})


GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)

add_library(consumer_lib_objects OBJECT ${SOURCE_FILES})
target_include_directories(consumer_lib_objects PRIVATE ../c/include include ${ASAPO_CXX_COMMON_INCLUDE_DIR} )
target_include_directories(consumer_lib_objects SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR})


if (BUILD_STATIC_CLIENT_LIBS)
    add_library(${TARGET_NAME} STATIC $<TARGET_OBJECTS:consumer_lib_objects>  $<TARGET_OBJECTS:asapo_fabric_objects> $<TARGET_OBJECTS:system_io>
            $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>  $<TARGET_OBJECTS:curl_http_client> )
    target_include_directories(${TARGET_NAME} SYSTEM PUBLIC $<BUILD_INTERFACE:${LIBFABRIC_INCLUDE_DIR}>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../c/include>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${ASAPO_CXX_COMMON_INCLUDE_DIR}>)
    target_include_directories(${TARGET_NAME} INTERFACE $<INSTALL_INTERFACE:include>)
    target_link_libraries(${TARGET_NAME} INTERFACE CURL::libcurl ${CMAKE_THREAD_LIBS_INIT} ${ASAPO_COMMON_FABRIC_LIBRARIES})
    #install
    install(TARGETS  ${TARGET_NAME} EXPORT AsapoConsumerStaticTarget
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include
            )


    export(EXPORT AsapoConsumerStaticTarget
            FILE "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoConsumerStaticTarget.cmake"
            NAMESPACE imported::
            )
    install(EXPORT AsapoConsumerStaticTarget
            FILE AsapoConsumerStaticTarget.cmake
            NAMESPACE imported::
            DESTINATION ${ConfigPackageLocation}
            )


endif()

if (BUILD_SHARED_CLIENT_LIBS)
    add_library(${TARGET_NAME}_shared SHARED $<TARGET_OBJECTS:consumer_lib_objects>  $<TARGET_OBJECTS:asapo_fabric_objects> $<TARGET_OBJECTS:system_io>
            $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>  $<TARGET_OBJECTS:curl_http_client> )
    target_include_directories(${TARGET_NAME}_shared SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR})
    target_include_directories(${TARGET_NAME}_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${ASAPO_CXX_COMMON_INCLUDE_DIR})

    target_link_libraries(${TARGET_NAME}_shared CURL::libcurl ${CMAKE_THREAD_LIBS_INIT} ${ASAPO_COMMON_FABRIC_LIBRARIES})
    IF(NOT WIN32)
        set_target_properties(${TARGET_NAME}_shared PROPERTIES OUTPUT_NAME ${TARGET_NAME})
        set_target_properties(${TARGET_NAME}_shared PROPERTIES VERSION ${CONSUMER_CLIENT_VER}
                                           SOVERSION ${CONSUMER_CLIENT_VER})
        set_target_properties(${TARGET_NAME}_shared PROPERTIES EXPORT_NAME ${TARGET_NAME})
    ENDIF()
    target_include_directories(${TARGET_NAME}_shared INTERFACE $<INSTALL_INTERFACE:include>)

    install(TARGETS  ${TARGET_NAME}_shared EXPORT AsapoConsumerSharedTarget
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include
            )
    export(EXPORT AsapoConsumerSharedTarget
            FILE "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoConsumerSharedTarget.cmake"
            NAMESPACE imported::
            )
    install(EXPORT AsapoConsumerSharedTarget
            FILE
            AsapoConsumerSharedTarget.cmake
            NAMESPACE imported::
            DESTINATION ${ConfigPackageLocation}
            )
endif()

install(DIRECTORY include/ DESTINATION include)

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

