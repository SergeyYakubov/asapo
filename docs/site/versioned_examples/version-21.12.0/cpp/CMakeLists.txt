cmake_minimum_required(VERSION 3.3)

project(asapo-client)

set(CMAKE_CXX_STANDARD 11)

# optionally use some other curl lib (e.g. static)
# set (CURL_LIBRARY /usr/local/lib/libasapo-curl.a)
# optionally linh gcc and stdc++ statically
# set (ASAPO_STATIC_CXX_LIBS ON)
# optionally link asapo as shared libs (ASAPO_STATIC_CXX_LIBS not used then)
# set (ASAPO_SHARED_LIBS ON)

#consumer snippet_start_remove
find_package (Asapo REQUIRED COMPONENTS Producer)

set(TARGET_NAME asapo-produce)
set(SOURCE_FILES produce.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} imported::asapo-producer)
#consumer snippet_end_remove
#producer snippet_start_remove
find_package (Asapo REQUIRED COMPONENTS Consumer)

set(TARGET_NAME asapo-consume)
set(SOURCE_FILES consume.cpp)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} imported::asapo-consumer)
#producer snippet_end_remove