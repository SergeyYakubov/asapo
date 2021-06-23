cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME "asapo-consume-c")

set(SOURCE_FILES consume.c)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} asapo-consumer ${CMAKE_THREAD_LIBS_INIT})
