set(TARGET_NAME asapo-authorizer)

################################
# Testing
################################
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/asap3 ASAP3_FOLDER )
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/beamline CURRENT_BEAMLINES_FOLDER )

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ip_bl_mapping BEAMLINES_FOLDER )
if (WIN32)
    string(REPLACE "\\" "\\\\" ASAP3_FOLDER "${ASAP3_FOLDER}")
    string(REPLACE "\\" "\\\\" BEAMLINES_FOLDER "${BEAMLINES_FOLDER}")
    string(REPLACE "\\" "\\\\" CURRENT_BEAMLINES_FOLDER "${CURRENT_BEAMLINES_FOLDER}")

endif()

configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/auth_secret.key auth_secret.key COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/auth_secret_admin.key auth_secret_admin.key COPYONLY)

configure_file(beamtime-metadata-11111111.json beamtime-metadata-11111111.json COPYONLY)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/settings.json.in settings.json @ONLY)
add_script_test("${TARGET_NAME}-authorize" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>" nomem
        )
