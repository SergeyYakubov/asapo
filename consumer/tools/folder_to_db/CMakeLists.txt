set(TARGET_NAME folder2db)

set(SOURCE_FILES src/folder_db_importer.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>)
target_link_libraries(${TARGET_NAME} database)

add_executable(${TARGET_NAME}-bin src/main.cpp)
set_target_properties(${TARGET_NAME}-bin
        PROPERTIES OUTPUT_NAME ${TARGET_NAME})
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_folder_to_db.cpp)
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

