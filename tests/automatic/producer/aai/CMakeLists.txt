set(TARGET_NAME producer_python_aai)

################################
# Testing
################################
prepare_asapo()

prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_producer BINARY_DIR)
endif()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/beamline CURRENT_BEAMLINES_FOLDER )

if (WIN32)
    string(REPLACE "\\" "\\\\" CURRENT_BEAMLINES_FOLDER "${CURRENT_BEAMLINES_FOLDER}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/settings.json.tpl.in authorizer.json.tpl @ONLY)
configure_file(beamtime-metadata-11111111.json beamline/p07/current/beamtime-metadata-11111111.json COPYONLY)

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/producer_aai.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}-python" "${Python_EXECUTABLE} ${PYTHON_LIBS} ${TEST_SCRIPT} " nomem)



