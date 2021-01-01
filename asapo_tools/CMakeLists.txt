set (TARGET_NAME asapo)

if (NOT "$ENV{GOPATH}" STREQUAL "")
	set(GOPATH $ENV{GOPATH})
endif()

if (NOT GOPATH)
    message (FATAL_ERROR "GOPATH not set")
endif()

message(STATUS "global gopath ${GOPATH}")

IF(WIN32)
    set (gopath "${GOPATH}\;${CMAKE_CURRENT_SOURCE_DIR}\;${CMAKE_SOURCE_DIR}/common/go")
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (gopath ${GOPATH}:${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_SOURCE_DIR}/common/go)
    set (exe_name "${TARGET_NAME}")
#    set (GO_OPTS "GOOS=linux;CGO_ENABLED=0")
ENDIF()

include(testing_go)

add_custom_target(asapo ALL
    COMMAND  ${CMAKE_COMMAND} -E env GOPATH=${gopath}
        ${GO_OPTS} go build -o ${exe_name} asapo_tools/main
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(${TARGET_NAME} PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})

gotest(${TARGET_NAME}  "${CMAKE_CURRENT_SOURCE_DIR}" "./...")
