set(TARGET_NAME full_chain_simple_chain_raw)

################################
# Testing
################################
prepare_asapo()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/asap3 ASAP3_FOLDER )
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/beamline CURRENT_BEAMLINES_FOLDER )

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ip_bl_mapping BEAMLINES_FOLDER )
if (WIN32)
    string(REPLACE "\\" "\\\\" ASAP3_FOLDER "${ASAP3_FOLDER}")
    string(REPLACE "\\" "\\\\" BEAMLINES_FOLDER "${BEAMLINES_FOLDER}")
    string(REPLACE "\\" "\\\\" CURRENT_BEAMLINES_FOLDER "${CURRENT_BEAMLINES_FOLDER}")

endif()

configure_file(beamtime-metadata-11111111.json beamtime-metadata-11111111.json COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/settings.json.tpl.in authorizer.json.tpl @ONLY)

add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
