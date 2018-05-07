set(TARGET_NAME resolve_hostname_to_ip)
set(SOURCE_FILES resolve_hostname_to_ip.cpp)

################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> )
target_link_libraries(${TARGET_NAME} test_common)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_integration_test(${TARGET_NAME} resolve_hostname_to_ip "")
