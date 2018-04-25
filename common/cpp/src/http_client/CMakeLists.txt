set(TARGET_NAME curl_http_client)
set(SOURCE_FILES
        curl_http_client.cpp
        http_client_factory.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
IF(WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC -DCURL_STATICLIB)
ENDIF()
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})


