configure_file(setup.py.in setup.py @ONLY)

ADD_CUSTOM_TARGET(windows-bdist ALL
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
        )


ADD_CUSTOM_TARGET(copy_python_bdist ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/asapo_consumer
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:asapo_consumer> ${CMAKE_CURRENT_BINARY_DIR}/asapo_consumer/
        )


ADD_DEPENDENCIES(copy_python_bdist asapo_consumer)

ADD_DEPENDENCIES(windows-bdist copy_python_bdist)
