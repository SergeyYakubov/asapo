set(TARGET_NAME curl_httpclient_command)
set(SOURCE_FILES curl_httpclient_command.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common hidra2-worker)

#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#if (CMAKE_COMPILER_IS_GNUCXX)
#    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
#endif()


################################
# Testing
################################

#add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME}  get_google "GET google.com moved 302")
add_integration_test(${TARGET_NAME}  get_badaddress "GET google.com/badaddress found 404")
add_integration_test(${TARGET_NAME}  get_badaddress2 "GET 111 clienterror 404")
add_integration_test(${TARGET_NAME}  post "POST httpbin.org/post testdata 200")
add_integration_test(${TARGET_NAME}  post_badaddress "POST google.com/badaddress found 404")
add_integration_test(${TARGET_NAME}  post_badaddress2 "POST 111 clienterror 404")