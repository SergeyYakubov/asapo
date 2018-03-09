add_subdirectory(src/system_io)

add_subdirectory(src/json_parser)

add_subdirectory(src/data_structs)

if(BUILD_MONGODB_CLIENTLIB)
    add_subdirectory(src/database)
endif()

install(DIRECTORY ${HIDRA2_CXX_COMMON_INCLUDE_DIR}/common
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include )

