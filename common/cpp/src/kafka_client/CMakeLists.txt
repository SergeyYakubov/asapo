set(TARGET_NAME rdkafka_client)
set(SOURCE_FILES
        rdkafka_client.cpp
        kafka_client_factory.cpp ../../include/asapo/preprocessor/definitions.h)


################################
# Library
################################
add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
IF(WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC -DRDKAFKA_STATICLIB)
ENDIF()
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR} ${RDKAFKA_INCLUDE_DIRS})


