set(TARGET_NAME database)
set(SOURCE_FILES
        mongodb_client.cpp
        database.cpp)

################################
# Library
################################
find_package (libmongoc-static-1.0 1.9 REQUIRED)
message ("--   mongoc found version \"${MONGOC_STATIC_VERSION}\"")
message ("--   mongoc include path \"${MONGOC_STATIC_INCLUDE_DIRS}\"")
message ("--   mongoc libraries \"${MONGOC_STATIC_LIBRARIES}\"")


add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:data_structs>)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR}
        PUBLIC "${MONGOC_STATIC_INCLUDE_DIRS}")
target_link_libraries (${TARGET_NAME} PRIVATE "${MONGOC_STATIC_LIBRARIES}")
target_compile_definitions (${TARGET_NAME} PRIVATE "${MONGOC_STATIC_DEFINITIONS}")
