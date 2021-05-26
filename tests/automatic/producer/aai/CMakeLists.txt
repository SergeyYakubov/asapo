set(TARGET_NAME producer_python_aai)

################################
# Testing
################################

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_producer BINARY_DIR)
endif()

configure_file(beamtime-metadata-11111111.json beamtime-metadata-11111111.json)

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/producer_aai.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}-python" "${Python_EXECUTABLE} ${PYTHON_LIBS} ${TEST_SCRIPT} " nomem)



