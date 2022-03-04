set(TARGET_NAME python_deadlock_producer)


find_package (Python3 REQUIRED)

if (UNIX)
    get_target_property(PYTHON_LIBS_PRODUCER python-lib-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS_PRODUCER asapo_producer BINARY_DIR)
endif()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/producer_segfault.py TEST_SCRIPT )


add_script_test("${TARGET_NAME}" "${TEST_SCRIPT} ${PYTHON_LIBS_PRODUCER} ${Python3_EXECUTABLE}" nomem)


