configure_file(setup.py.in setup.py @ONLY)

ADD_CUSTOM_TARGET(windows-bdist ALL
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
        )


ADD_CUSTOM_TARGET(copy_python_bdist ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/asapo_worker
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_worker.pyd ${CMAKE_CURRENT_BINARY_DIR}/asapo_worker/
        )


ADD_DEPENDENCIES(copy_python_bdist asapo_worker)

ADD_DEPENDENCIES(windows-bdist copy_python_bdist)
