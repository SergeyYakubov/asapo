set(TARGET_NAME dummy-data-producer)
set(SOURCE_FILES
        dummy_data_producer.cpp
        )

if(BUILD_EXAMPLES)


add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} asapo-producer)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()

add_script_test("${TARGET_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")

endif()

set (dir share/asapo/examples/${TARGET_NAME})
install(FILES ${SOURCE_FILES} DESTINATION "${dir}")

configure_file(CMakeLists_separate.in CMakeLists_separate.txt @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists_separate.txt DESTINATION "${dir}" RENAME CMakeLists.txt)

configure_file(Makefile.in Makefile_LINUX @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile_LINUX DESTINATION "${dir}")


