set(TARGET_NAME full_chain_two_beamlines)

################################
# Testing
################################
prepare_asapo()

add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)
if (ENABLE_LIBFABRIC)
    if (ENABLE_LIBFABRIC_LOCALHOST)
        add_script_test("${TARGET_NAME}-fabric" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> fabric" nomem)
    else ()
        message(WARNING "Disabled automated LibFabric of '${TARGET_NAME}-fabric' test because 'ENABLE_LIBFABRIC_LOCALHOST' is not enabled.")
    endif()
endif()

