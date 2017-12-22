set(TARGET_NAME worker_processfolder)
set(SOURCE_FILES process_folder.cpp)


################################
# Executable and link
################################
#if (CMAKE_BUILD_TYPE STREQUAL "Release")
#set(CMAKE_FIND_STATIC FIRST)
#endif()
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} common worker-api ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#use expression generator to get rid of VS adding Debug/Release folders
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

get_target_property(VAR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
message (STATUS  ${VAR})

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()
#set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "-static-libstdc++")

add_example_test("${TARGET_NAME}")
