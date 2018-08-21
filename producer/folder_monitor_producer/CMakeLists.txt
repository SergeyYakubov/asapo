set(TARGET_NAME folder-monitor-producer)
set(SOURCE_FILES
        src/event_detector.cpp
        )


################################
# Library (needed for unit tests)
################################

add_library(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} producer-api)



################################
# Executable
################################
add_executable(${TARGET_NAME}-bin src/foldermon_main.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME asapo-foldermon-producer)
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

set_target_properties(${TARGET_NAME}-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )


################################
# Testing
################################
set(TEST_SOURCE_FILES
        unittests/test_foldermon_config.cpp
   )

set(TEST_LIBRARIES "${TARGET_NAME}")

#gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

install(TARGETS ${TARGET_NAME}-bin DESTINATION bin)

