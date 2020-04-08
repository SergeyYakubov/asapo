add_subdirectory(automatic)


configure_files(${CMAKE_CURRENT_SOURCE_DIR}/manual/tests_via_nomad ${CMAKE_CURRENT_BINARY_DIR}/manual/tests_via_nomad @ONLY)

add_subdirectory(manual/performance_broker_receiver)

add_subdirectory(manual/asapo_fabric)
