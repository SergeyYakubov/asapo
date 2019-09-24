set(TARGET_NAME getnext_broker_python)


prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib2 BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" ${PYTHON_LIBS} nomem)

configure_file(getnext.py getnext.py COPYONLY)

set (dir examples/consumer/${TARGET_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/getnext.py DESTINATION "${dir}")
