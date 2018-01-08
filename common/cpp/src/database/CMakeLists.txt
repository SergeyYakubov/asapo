set(TARGET_NAME mongo_db)
set(SOURCE_FILES
        mongodb_client.cpp)

################################
# Library
################################
add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})


