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
#set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "-static-libstdc++")

add_example_test("${TARGET_NAME}")
