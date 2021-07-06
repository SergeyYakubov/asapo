# Automatically add all files to the tests
file(GLOB files "*.cpp")
foreach(file ${files})
    # File name without extension
    get_filename_component(file_we "${file}" NAME_WE)
    set(TARGET_NAME test-auto-asapo_fabric-${file_we})
    set(SOURCE_FILES ${file})

    GET_PROPERTY(ASAPO_COMMON_FABRIC_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES)

    # Executable and link
    add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:logger> $<TARGET_OBJECTS:curl_http_client>)
    target_link_libraries(${TARGET_NAME} test_common asapo-fabric CURL::libcurl ${ASAPO_COMMON_FABRIC_LIBRARIES})
    target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
    target_include_directories(${TARGET_NAME} SYSTEM PUBLIC  ${LIBFABRIC_INCLUDE_DIR})
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    # Add test
    if (file_we STREQUAL "timeout_test")
        add_integration_test(${TARGET_NAME} ${TARGET_NAME} "" nomem)
    else()
        add_integration_test(${TARGET_NAME} ${TARGET_NAME} "")
    endif ()


endforeach()
