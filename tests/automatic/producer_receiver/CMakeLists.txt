add_subdirectory(transfer_single_file)
add_subdirectory(transfer_single_file_bypass_buffer)
add_subdirectory(transfer_datasets)

#if (UNIX) #removed since monitoring willbe refactored anyway
#    add_subdirectory(check_monitoring)
#endif()