set(TARGET_NAME full_chain_two_beamlines)

################################
# Testing
################################
add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver_tcp.json.tpl.lin.in receiver_tcp.json.tpl @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/admin_token.key admin_token.key COPYONLY)

if (ENABLE_LIBFABRIC)
    if (ENABLE_LIBFABRIC_LOCALHOST)
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver_fabric.json.tpl.lin.in receiver_fabric.json.tpl @ONLY)
        add_script_test("${TARGET_NAME}-fabric" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> fabric" nomem)
    else ()
        message(WARNING "Disabled automated LibFabric of '${TARGET_NAME}-fabric' test because 'ENABLE_LIBFABRIC_LOCALHOST' is not enabled.")
    endif()
endif()

