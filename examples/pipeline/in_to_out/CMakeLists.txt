set(TARGET_NAME pipeline_inout)
set(SOURCE_FILES in_to_out.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} asapo-worker producer-api)

#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

prepare_asapo()

add_script_test("${TARGET_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}" nomem)

