if(BUILD_EXAMPLES)
    add_subdirectory(pipeline)
endif()

if(BUILD_EXAMPLES OR INSTALL_EXAMPLES)
    add_subdirectory(consumer)
endif()

if(BUILD_EXAMPLES OR INSTALL_EXAMPLES OR BUILD_INTEGRATION_TESTS)
    add_subdirectory(producer)
endif()