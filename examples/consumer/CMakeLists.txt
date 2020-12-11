find_package(Threads)

add_subdirectory(getnext_broker)

if(BUILD_EXAMPLES AND BUILD_PYTHON)
        add_subdirectory(getnext_broker_python)
endif()


