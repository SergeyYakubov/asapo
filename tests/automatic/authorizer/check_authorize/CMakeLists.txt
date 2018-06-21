set(TARGET_NAME asapo-authorizer)

################################
# Testing
################################
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/OpenBeamTimes.txt BEAMTIMES_FILE )
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ip_bl_mapping BEAMLINES_FOLDER )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/settings.json.in settings.json @ONLY)
add_script_test("${TARGET_NAME}-authorize" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>" nomem
        )
