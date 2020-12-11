configure_file(setup.py.in setup.py @ONLY)
configure_file(MANIFEST.in MANIFEST.in @ONLY)

if ("source" IN_LIST BUILD_PYTHON_PACKAGES )
    ADD_CUSTOM_TARGET(python-dist-consumer ALL
        COMMAND ${Python_EXECUTABLE} setup.py sdist
        )
    ADD_DEPENDENCIES(python-dist-consumer copy_python_dist-consumer)
endif()

if ("rpm" IN_LIST BUILD_PYTHON_PACKAGES)
    ADD_CUSTOM_TARGET(python-rpm-consumer ALL
            COMMAND PACKAGE_PREFIX=python- python setup.py bdist_rpm --release=1.${PACKAGE_RELEASE_SUFFIX} --requires=numpy
            )

    ADD_CUSTOM_TARGET(python3-rpm-consumer ALL
            COMMAND PACKAGE_PREFIX=python3- python3 setup.py bdist_rpm --release=1.${PACKAGE_RELEASE_SUFFIX} --requires=python3-numpy
            )
    ADD_DEPENDENCIES(python-rpm-consumer copy_python_dist-consumer)
    ADD_DEPENDENCIES(python3-rpm-consumer copy_python_dist-consumer)
endif()

ADD_CUSTOM_TARGET(copy_python_dist-consumer ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_wrappers.h ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_consumer.cpp ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/consumer/api/cpp/include ${CMAKE_CURRENT_BINARY_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/common ${CMAKE_CURRENT_BINARY_DIR}/include/asapo/common
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/include/common/version.h.in
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:asapo-consumer> ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:asapo-fabric> ${CMAKE_CURRENT_BINARY_DIR}/lib

        )

ADD_DEPENDENCIES(copy_python_dist-consumer python-lib-consumer)
