set(TARGET_NAME folder2db)

set(SOURCE_FILES src/FolderDBConverter.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:common>)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/common/cpp/include)


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_folder_to_db.cpp)
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
