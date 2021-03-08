set (TARGET_NAME asapo-broker)

IF(WIN32)
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (exe_name "${TARGET_NAME}")
ENDIF()

include(testing_go)

configure_file(docker/Dockerfile . COPYONLY)

add_custom_target(asapo-broker ALL
    COMMAND go build ${GO_OPTS} -o ${exe_name} main/broker.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_broker
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(asapo-broker PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})

gotest(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "./...")
go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
go_integration_test(${TARGET_NAME}-nextrecord "./..." "MongoDBNext")
