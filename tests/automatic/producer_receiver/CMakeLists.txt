add_subdirectory(transfer_single_file)
add_subdirectory(transfer_datasets)

if (UNIX)
    add_subdirectory(check_monitoring)
endif()