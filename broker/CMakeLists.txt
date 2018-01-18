set (TARGET_NAME hidra2-broker)

if (NOT "$ENV{GOPATH}" STREQUAL "")
	set(GOPATH $ENV{GOPATH})
endif()

if (NOT GOPATH)
    message (FATAL_ERROR "GOPATH not set")
endif()

message(STATUS "global gopath ${GOPATH}")

IF(WIN32)
    set (gopath "${GOPATH}\;${CMAKE_CURRENT_SOURCE_DIR}")
ELSE()
    set (gopath ${GOPATH}:${CMAKE_CURRENT_SOURCE_DIR})
ENDIF()

include(testing_go)

add_custom_target(hidra2-broker ALL
    COMMAND  ${CMAKE_COMMAND} -E env GOPATH=${gopath}
    go build ${GO_OPTS} -o ${TARGET_NAME} hidra2_broker/main
    VERBATIM)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} DESTINATION bin)

gotest(${TARGET_NAME} "./...")
go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
go_integration_test(${TARGET_NAME}-nextrecord "./..." "MongoDBNext")
