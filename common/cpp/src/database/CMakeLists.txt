set(TARGET_NAME database)
set(SOURCE_FILES
        mongodb_client.cpp
        encoding.cpp
        database.cpp)

################################
# Library
################################
find_package (libmongoc-static-1.0 1.15 REQUIRED)
message ("--   mongoc found version \"${MONGOC_STATIC_VERSION}\"")
message ("--   mongoc include path \"${MONGOC_STATIC_INCLUDE_DIRS}\"")
message ("--   mongoc libraries \"${MONGOC_STATIC_LIBRARIES}\"")

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:data_structs> $<TARGET_OBJECTS:json_parser> $<TARGET_OBJECTS:system_io>)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR}
        PUBLIC "${MONGOC_STATIC_INCLUDE_DIRS}")
target_link_libraries (${TARGET_NAME} PRIVATE "${MONGOC_STATIC_LIBRARIES}")
target_compile_definitions (${TARGET_NAME} PRIVATE "${MONGOC_STATIC_DEFINITIONS}")


################################
# Testing
################################
set(TEST_SOURCE_FILES ../../unittests/database/test_encoding.cpp)

set(TEST_LIBRARIES "${TARGET_NAME}")

include_directories(${ASAPO_CXX_COMMON_INCLUDE_DIR})
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" "nocov")

