add_subdirectory(api/cpp)

if (BUILD_PYTHON)
    add_subdirectory(api/python)
endif()

if(BUILD_EVENT_MONITOR_PRODUCER)
    add_subdirectory(event_monitor_producer)
endif(BUILD_EVENT_MONITOR_PRODUCER)

include(GNUInstallDirs)
configure_file(libasapo-producer.pc.in libasapo-producer.pc)
install(FILES ${CMAKE_BINARY_DIR}/producer/libasapo-producer.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
