set(TARGET_NAME hidra2-worker)

set(SOURCE_FILES
        src/data_broker.cpp
        src/http_client.cpp
        src/folder_data_broker.cpp
        src/server_data_broker.cpp
        src/curl_http_client.cpp
        src/http_error.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>
        $<TARGET_OBJECTS:data_structs>)

set (CMAKE_PREFIX_PATH "${LIBCURL_DIR}")
find_package (CURL REQUIRED)
message (STATUS "Found cURL libraries: ${CURL_LIBRARIES}")
message (STATUS "cURL include: ${CURL_INCLUDE_DIRS}")


target_include_directories(${TARGET_NAME} PUBLIC include ${HIDRA2_CXX_COMMON_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS})

IF(WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC -DCURL_STATICLIB)
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
ENDIF(WIN32)

target_link_libraries(${TARGET_NAME} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_worker_api.cpp
                      unittests/test_folder_broker.cpp
                      unittests/test_server_broker.cpp)
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

install(TARGETS ${TARGET_NAME} DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)
