set(TARGET_NAME curl_httpclient_get)
set(SOURCE_FILES curl_httpclient_get.cpp)


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
add_integration_test(${TARGET_NAME} get_google "google.com moved 302")
add_integration_test(${TARGET_NAME} get_badaddress "google.com/badaddress found 404")
add_integration_test(${TARGET_NAME} get_badaddress2 "111 clienterror 404")

