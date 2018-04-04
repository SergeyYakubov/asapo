set(TARGET_NAME dummy-data-producer)
set(SOURCE_FILES
        dummy_data_producer.cpp
        )

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${HIDRA2_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} producer-api)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()

set (dir examples/${TARGET_NAME})
install(TARGETS ${TARGET_NAME} DESTINATION "${dir}")
install(FILES ${SOURCE_FILES} DESTINATION "${dir}")

configure_file(CMakeLists_separate.in CMakeLists_separate.txt @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists_separate.txt DESTINATION "${dir}" RENAME CMakeLists.txt)

configure_file(Makefile.in Makefile_LINUX @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile_LINUX DESTINATION "${dir}")


add_script_test("${TARGET_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
