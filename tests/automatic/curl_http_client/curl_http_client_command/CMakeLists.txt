set(TARGET_NAME curl_httpclient_command)
set(SOURCE_FILES curl_httpclient_command.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-consumer)

#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#if (CMAKE_COMPILER_IS_GNUCXX)
#    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
#endif()


################################
# Testing
################################

prepare_asapo()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/authorizer_settings.json.tpl.in authorizer.json.tpl @ONLY)

add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>" nomem)
