set(CMAKE_CXX_STANDARD 11)
IF(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    add_definitions(-DWIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -pedantic -Werror -Wconversion -fPIC)
    add_link_options(-static-libgcc -static-libstdc++)
ELSEIF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Werror -Wconversion)
ENDIF(WIN32)

set (ASAPO_CXX_COMMON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/common/cpp/include)

find_package (Threads)

#TODO: Better way then GLOBAL PROPERTY
IF(WIN32)
    find_package(Threads REQUIRED)
    SET(ASAPO_COMMON_IO_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32)
ELSEIF(UNIX)
    SET(ASAPO_COMMON_IO_LIBRARIES Threads::Threads)
ENDIF(WIN32)
SET_PROPERTY(GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES ${ASAPO_COMMON_IO_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DUNIT_TESTS)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

if (APPLE)
    link_directories("/usr/local/lib")
endif()