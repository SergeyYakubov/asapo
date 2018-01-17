set (TARGET_NAME hidra2-broker)

set (gopath $ENV{GOPATH}:${CMAKE_CURRENT_SOURCE_DIR})

include(testing_go)

message(STATUS ${gopath})
add_custom_target(hidra2-broker ALL
				  COMMAND  GOPATH=${gopath} go build ${GO_OPTS} -o ${TARGET_NAME} hidra2_broker/main
				  VERBATIM)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} DESTINATION bin)

gotest(${TARGET_NAME} "./...")
go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
