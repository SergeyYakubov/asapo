if (UNIX)
    add_subdirectory(receiver_cpu_usage)
    add_subdirectory(consumer_python_memleak)
    add_subdirectory(error-sending-data-using-callback-method)
endif()

add_subdirectory(producer_send_after_restart)
