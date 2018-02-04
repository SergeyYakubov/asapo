set(TARGET_NAME getnext_broker)
set(SOURCE_FILES getnext_broker.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} hidra2-worker)
#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

get_target_property(VAR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)

add_script_test("${TARGET_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} $<TARGET_PROPERTY:hidra2-broker,EXENAME>")

set (dir examples/worker/${TARGET_NAME})
install(TARGETS ${TARGET_NAME} DESTINATION "${dir}")
install(FILES ${SOURCE_FILES} DESTINATION "${dir}")

configure_file(CMakeLists_separate.in CMakeLists_separate.txt @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists_separate.txt DESTINATION "${dir}" RENAME CMakeLists.txt)

configure_file(Makefile.in Makefile_LINUX @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile_LINUX DESTINATION "${dir}")
