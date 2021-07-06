set(TARGET_NAME full_chain_simple_chain_withdata_fromcache)

################################
# Testing
################################
if (UNIX)
    set (ROOT_PATH "/tmp/asapo/")
else()
    set (ROOT_PATH "c:\\\\tmp\\\\asapo\\\\")
endif()

configure_file(test.json.in test.json @ONLY)

prepare_asapo()

add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)

if (ENABLE_LIBFABRIC)
    if (ENABLE_LIBFABRIC_LOCALHOST)
        add_script_test("${TARGET_NAME}-fabric" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> fabric" nomem)
    else ()
        message(WARNING "Disabled automated LibFabric of '${TARGET_NAME}-fabric' test because 'ENABLE_LIBFABRIC_LOCALHOST' is not enabled.")
    endif()
endif()
