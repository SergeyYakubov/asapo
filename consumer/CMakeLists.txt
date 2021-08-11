add_subdirectory(api/cpp)
add_subdirectory(api/c)


if (BUILD_PYTHON)
    add_subdirectory(api/python)
endif()


if(BUILD_CONSUMER_TOOLS)
    add_subdirectory(tools)
endif()

configure_file(libasapo-consumer.pc.in libasapo-consumer.pc)
install(FILES ${CMAKE_BINARY_DIR}/consumer/libasapo-consumer.pc
        DESTINATION lib/pkgconfig)
