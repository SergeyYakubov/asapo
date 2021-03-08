set (TARGET_NAME asapo-authorizer)

IF(WIN32)
    set (exe_name "${TARGET_NAME}.exe")
ELSE()
    set (exe_name "${TARGET_NAME}")
ENDIF()

include(testing_go)

configure_file(docker/Dockerfile . COPYONLY)

add_custom_target(asapo-authorizer ALL
    COMMAND go build ${GO_OPTS} -o ${CMAKE_CURRENT_BINARY_DIR}/${exe_name} main/authorizer.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/asapo_authorizer
    VERBATIM)
define_property(TARGET PROPERTY EXENAME
        BRIEF_DOCS <executable name>
        FULL_DOCS <full-doc>)

set_target_properties(asapo-authorizer PROPERTIES EXENAME ${CMAKE_CURRENT_BINARY_DIR}/${exe_name})

gotest(${TARGET_NAME}  "${CMAKE_CURRENT_SOURCE_DIR}" "./...")
#go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
#go_integration_test(${TARGET_NAME}-nextrecord "./..." "MongoDBNext")
