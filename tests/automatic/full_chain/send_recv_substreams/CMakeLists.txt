set(TARGET_NAME send_recv_substreams)
set(SOURCE_FILES send_recv_substreams.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} asapo-consumer asapo-producer)

#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

prepare_asapo()

add_script_test("${TARGET_NAME}-tcp" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} tcp" nomem)

