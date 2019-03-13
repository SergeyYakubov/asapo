set(TARGET_NAME getnext_broker_python)


prepare_asapo()

get_target_property(PYTHON_LIBS python-lib2 BINARY_DIR)


add_script_test("${TARGET_NAME}" ${PYTHON_LIBS} nomem)

configure_file(getnext.py getnext.py COPYONLY)

set (dir examples/worker/${TARGET_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/getnext.py DESTINATION "${dir}")
