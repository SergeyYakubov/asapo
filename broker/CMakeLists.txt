set (TARGET_NAME asapo-broker)

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
ENDIF()

include(testing_go)

add_custom_target(asapo-broker ALL
    COMMAND  ${CMAKE_COMMAND} -E env GOPATH=${gopath}
    go build ${GO_OPTS} -o ${exe_name} asapo_broker/main
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(asapo-broker PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})


install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} DESTINATION bin)

gotest(${TARGET_NAME} "./...")
go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
go_integration_test(${TARGET_NAME}-nextrecord "./..." "MongoDBNext")
