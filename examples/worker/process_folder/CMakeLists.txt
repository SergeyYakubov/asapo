set(TARGET_NAME worker_processfolder)
set(SOURCE_FILES process_folder.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})

#Add all necessary common libraries
GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} asapo-worker)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

get_target_property(VAR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()

add_script_test("${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME})


set (dir examples/worker/process_folder)
install(TARGETS ${TARGET_NAME} DESTINATION "${dir}")
install(FILES ${SOURCE_FILES} DESTINATION "${dir}")

configure_file(CMakeLists_separate.in CMakeLists_separate.txt @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists_separate.txt DESTINATION "${dir}" RENAME CMakeLists.txt)

configure_file(Makefile.in Makefile_LINUX @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile_LINUX DESTINATION "${dir}")
