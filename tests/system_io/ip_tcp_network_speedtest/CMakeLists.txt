set(TARGET_NAME ip_tcp_network_speedtest)
set(SOURCE_FILES ip_tcp_network_speedtest.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:common>)

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${HIDRA2_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} test_common)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/common/cpp/include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_integration_test(${TARGET_NAME} ip_tcp_network_speedtest "")
