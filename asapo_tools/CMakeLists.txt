set (TARGET_NAME asapo)

IF(WIN32)
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (exe_name "${TARGET_NAME}")
#    set (GO_OPTS "GOOS=linux;CGO_ENABLED=0")
ENDIF()

include(testing_go)

add_custom_target(asapo ALL
    COMMAND  go build ${GO_OPTS} -o ${exe_name} main/asapo.go
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_tools
        VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(${TARGET_NAME} PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})

gotest(${TARGET_NAME}  "${CMAKE_CURRENT_SOURCE_DIR}" "./...")
