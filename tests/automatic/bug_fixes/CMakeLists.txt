if (UNIX)
    add_subdirectory(receiver_cpu_usage)
    add_subdirectory(worker_python_memleak)
endif()

add_subdirectory(producer_send_after_restart)