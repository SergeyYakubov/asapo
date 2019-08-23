set(TARGET_NAME producer_python_api)

################################
# Testing
################################
prepare_asapo()

prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib2-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_producer BINARY_DIR)
endif()


file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/producer_api.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}-python2" "python ${PYTHON_LIBS} ${TEST_SCRIPT} " nomem)

add_script_test("${TARGET_NAME}-python3" "python3 ${PYTHON_LIBS} ${TEST_SCRIPT} " nomem)


