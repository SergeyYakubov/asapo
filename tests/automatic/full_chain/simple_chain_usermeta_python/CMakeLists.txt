set(TARGET_NAME full_chain_usermeta_python)

################################
# Testing
################################

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-consumer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/admin_token.key admin_token.key COPYONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_PROPERTY:asapo,EXENAME> ${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_LIBS} ${Python_EXECUTABLE}" nomem)
