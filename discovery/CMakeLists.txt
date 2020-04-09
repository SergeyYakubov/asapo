set (TARGET_NAME asapo-discovery)

IF(WIN32)
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (exe_name "${TARGET_NAME}")
ENDIF()

include(testing_go)

configure_file(docker/Dockerfile . COPYONLY)

add_custom_target(${TARGET_NAME} ALL
    COMMAND go build ${GO_OPTS} -o ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} main/discovery.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_discovery
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(${TARGET_NAME} PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})


install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} DESTINATION bin)

gotest(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_discovery" "./...")
