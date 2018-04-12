set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/receiver.h src/receiver.cpp
        src/connection.h src/connection.cpp
        src/receiver_error.h
        src/request.cpp
        src/request_handler_file_write.cpp
        src/statistics.cpp
        src/statistics_sender_influx_db.cpp
        src/receiver_config.cpp src/receiver_config.h)


################################
# Library
################################


add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> $<TARGET_OBJECTS:curl_http_client>
         $<TARGET_OBJECTS:json_parser>)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


add_executable(${TARGET_NAME}-bin src/main.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME ${TARGET_NAME})
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

################################
# Testing
################################

set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)
#
set(TEST_SOURCE_FILES
        unittests/test_receiver.cpp
        unittests/test_connection.cpp
        unittests/test_statistics.cpp
        unittests/test_config.cpp
        unittests/test_request.cpp
        unittests/test_request_handler_file_write.cpp
        unittests/test_statistics_sender_influx_db.cpp
        unittests/mock_receiver_config.cpp
        )
#
set(TEST_LIBRARIES "${TARGET_NAME};system_io")
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
