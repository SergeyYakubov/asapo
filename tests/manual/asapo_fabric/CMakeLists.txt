GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)

add_executable(example-fabric-server fabric_server.cpp $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:curl_http_client>)
target_link_libraries(example-fabric-server asapo-fabric CURL::libcurl ${ASAPO_COMMON_FABRIC_LIBRARIES})

add_executable(example-fabric-client fabric_client.cpp)
target_link_libraries(example-fabric-client asapo-fabric ${ASAPO_COMMON_FABRIC_LIBRARIES})
