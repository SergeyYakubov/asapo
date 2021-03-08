set (TARGET_NAME asapo-file-transfer)

IF(WIN32)
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (exe_name "${TARGET_NAME}")
ENDIF()

include(testing_go)

configure_file(docker/Dockerfile . COPYONLY)

add_custom_target(asapo-file-transfer ALL
    COMMAND go build ${GO_OPTS} -o ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} main/file_transfer.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_file_transfer
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(asapo-file-transfer PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})

gotest(${TARGET_NAME}  "${CMAKE_CURRENT_SOURCE_DIR}" "./...")
