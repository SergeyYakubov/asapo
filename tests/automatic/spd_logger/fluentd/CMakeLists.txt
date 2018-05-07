set(TARGET_NAME spd_logger_fluentd)

set(SOURCE_FILES spd_logger_fluentd.cpp)

################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:curl_http_client>)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")
