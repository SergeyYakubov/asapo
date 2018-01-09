add_subdirectory(src/system_io)

if(BUILD_MONGODB_CLIENTLIB)
    add_subdirectory(src/database)
endif()

install(DIRECTORY ${HIDRA2_CXX_COMMON_INCLUDE_DIR}/common
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
