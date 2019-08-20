configure_file(setup.py.in setup.py @ONLY)

ADD_CUSTOM_TARGET(windows-bdist-producer ALL
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
        )


ADD_CUSTOM_TARGET(copy_python_bdist_producer ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/asapo_producer
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:asapo_producer> ${CMAKE_CURRENT_BINARY_DIR}/asapo_producer/
        )


ADD_DEPENDENCIES(copy_python_bdist_producer asapo_producer)

ADD_DEPENDENCIES(windows-bdist-producer copy_python_bdist_producer)
