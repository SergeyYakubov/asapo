cmake_minimum_required(VERSION 2.8)

project(asapo-produce)

set(CMAKE_CXX_STANDARD 11)

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
ENDIF()

find_package (Threads)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})

set(SOURCE_FILES produce.cpp)

link_directories(asapo/lib)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC asapo/include)
target_link_libraries(${TARGET_NAME} asapo-producer curl  ${CMAKE_THREAD_LIBS_INIT})
