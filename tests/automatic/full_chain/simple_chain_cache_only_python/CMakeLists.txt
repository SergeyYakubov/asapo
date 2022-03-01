set(TARGET_NAME simple-chaine-chache-only)


################################
# Testing
################################
if (UNIX)
    get_target_property(PYTHON_LIBS_PRODUCER python-lib-producer BINARY_DIR)
    get_target_property(PYTHON_LIBS_CONSUMER python-lib-consumer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS_PRODUCER asapo_producer BINARY_DIR)
    get_target_property(PYTHON_LIBS_CONSUMER asapo_consumer BINARY_DIR)
endif()


file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/transfer.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}-python" "${Python_EXECUTABLE} ${PYTHON_LIBS_PRODUCER} ${PYTHON_LIBS_CONSUMER} ${TEST_SCRIPT} " nomem)



