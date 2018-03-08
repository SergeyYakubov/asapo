add_subdirectory(src/system_io)

add_subdirectory(src/json_parser)

add_subdirectory(src/data_structs)

if(BUILD_MONGODB_CLIENTLIB)
    add_subdirectory(src/database)
endif()

install(DIRECTORY ${HIDRA2_CXX_COMMON_INCLUDE_DIR}/common
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include )

################################
# Library
################################
IF(WIN32)
    find_package(Threads REQUIRED)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32)
ELSEIF(UNIX)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES Threads::Threads)
ENDIF(WIN32)
