add_subdirectory(get_next)
add_subdirectory(get_last)
add_subdirectory(get_meta)

add_subdirectory(read_config)

if (UNIX)
add_subdirectory(check_monitoring)
endif()