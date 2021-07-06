find_package(Threads)

add_subdirectory(getnext)
add_subdirectory(simple-consumer-c)

if(BUILD_EXAMPLES AND BUILD_PYTHON)
        add_subdirectory(getnext_python)
endif()


