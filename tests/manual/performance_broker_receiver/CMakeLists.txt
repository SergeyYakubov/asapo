set(TARGET_NAME getlast_broker)
set(SOURCE_FILES getlast_broker.cpp)


add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} asapo-worker)

#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

get_target_property(VAR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
add_dependencies(${TARGET_NAME} asapo-broker)



