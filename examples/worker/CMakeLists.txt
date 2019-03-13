find_package(Threads)

add_subdirectory(process_folder)

add_subdirectory(getnext_broker)
IF (UNIX)
 add_subdirectory(getnext_broker_python)
ENDIF()
