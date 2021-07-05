set(TARGET_NAME asapo-producer)
set(SOURCE_FILES
        src/producer.cpp
        src/producer_impl.cpp
        src/producer_logger.cpp
        src/request_handler_tcp.cpp
        src/request_handler_filesystem.cpp
        src/receiver_discovery_service.cpp
        src/receiver_data_server_request_handler_factory.cpp
        src/producer_request.cpp)


string(REPLACE "v" "" PRODUCER_CLIENT_VER ${ASAPO_PRODUCER_PROTOCOL})


################################
# Library
################################
add_library(producer_lib_objects OBJECT ${SOURCE_FILES})
target_include_directories(producer_lib_objects PRIVATE include ${ASAPO_CXX_COMMON_INCLUDE_DIR} )
target_include_directories(producer_lib_objects SYSTEM PRIVATE ${LIBFABRIC_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})

if (BUILD_STATIC_CLIENT_LIBS)
    add_library(${TARGET_NAME} STATIC $<TARGET_OBJECTS:producer_lib_objects> $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:json_parser>
            $<TARGET_OBJECTS:curl_http_client> $<TARGET_OBJECTS:request_pool> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CURL_INCLUDE_DIRS}>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${ASAPO_CXX_COMMON_INCLUDE_DIR}>)
    target_include_directories(${TARGET_NAME} INTERFACE $<INSTALL_INTERFACE:include>)
    target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    #install
    install(TARGETS  ${TARGET_NAME} EXPORT AsapoProducerStaticTarget
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include
            )

    export(EXPORT AsapoProducerStaticTarget
            FILE "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoProducerStaticTarget.cmake"
            NAMESPACE imported::
            )
    install(EXPORT AsapoProducerStaticTarget
            FILE AsapoProducerStaticTarget.cmake
            NAMESPACE imported::
            DESTINATION ${ConfigPackageLocation}
            )
endif()

if (BUILD_SHARED_CLIENT_LIBS)
    add_library(${TARGET_NAME}_shared SHARED $<TARGET_OBJECTS:producer_lib_objects> $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:json_parser>
            $<TARGET_OBJECTS:curl_http_client> $<TARGET_OBJECTS:request_pool> $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:version>)
    target_include_directories(${TARGET_NAME}_shared SYSTEM PRIVATE ${CURL_INCLUDE_DIRS})
    target_include_directories(${TARGET_NAME}_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${ASAPO_CXX_COMMON_INCLUDE_DIR})

    target_link_libraries(${TARGET_NAME}_shared ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    IF(NOT WIN32)
        set_target_properties(${TARGET_NAME}_shared PROPERTIES OUTPUT_NAME ${TARGET_NAME})
        set_target_properties(${TARGET_NAME}_shared PROPERTIES VERSION ${PRODUCER_CLIENT_VER}
                SOVERSION ${PRODUCER_CLIENT_VER})
        set_target_properties(${TARGET_NAME}_shared PROPERTIES EXPORT_NAME ${TARGET_NAME})
    ENDIF()
    target_include_directories(${TARGET_NAME}_shared INTERFACE $<INSTALL_INTERFACE:include>)

    install(TARGETS  ${TARGET_NAME}_shared EXPORT AsapoProducerSharedTarget
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include
            )
    export(EXPORT AsapoProducerSharedTarget
            FILE "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoProducerSharedTarget.cmake"
            NAMESPACE imported::
            )
    install(EXPORT AsapoProducerSharedTarget
            FILE
            AsapoProducerSharedTarget.cmake
            NAMESPACE imported::
            DESTINATION ${ConfigPackageLocation}
            )
endif()

install(DIRECTORY include/ DESTINATION include)


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
