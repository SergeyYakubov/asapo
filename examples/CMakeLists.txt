add_subdirectory(consumer)

if(BUILD_EXAMPLES)
    add_subdirectory(pipeline)
endif()

add_subdirectory(producer)

if(BUILD_ASAPO_SITE)
    add_subdirectory(for_site)
endif()


