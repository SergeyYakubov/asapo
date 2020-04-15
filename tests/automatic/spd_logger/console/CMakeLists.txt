set(TARGET_NAME spd_logger_console)

set(SOURCE_FILES spd_logger_console.cpp)

################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:curl_http_client> $<TARGET_OBJECTS:system_io>)
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})


################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")
