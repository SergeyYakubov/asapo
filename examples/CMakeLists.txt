if(BUILD_EXAMPLES)
    add_subdirectory(pipeline)
endif()

if(BUILD_EXAMPLES OR INSTALL_EXAMPLES)
    add_subdirectory(producer)
    add_subdirectory(consumer)
endif()
