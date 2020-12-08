configure_file(setup.py.in setup.py @ONLY)
configure_file(MANIFEST.in MANIFEST.in @ONLY)


ADD_CUSTOM_TARGET(python-dist ALL
        COMMAND ${Python_EXECUTABLE} setup.py sdist
        )

ADD_CUSTOM_TARGET(python-rpm
        COMMAND PACKAGE_PREFIX=python- python setup.py bdist_rpm --release=${PACKAGE_RELEASE_SUFFIX}
        )

ADD_CUSTOM_TARGET(python3-rpm
        COMMAND PACKAGE_PREFIX=python3- python3 setup.py bdist_rpm --release=${PACKAGE_RELEASE_SUFFIX}
        )

#get_property(ASAPO_CONSUMER_LIB TARGET asapo-consumer PROPERTY LOCATION)

ADD_CUSTOM_TARGET(copy_python_dist ALL
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


ADD_DEPENDENCIES(copy_python_dist python-lib)

ADD_DEPENDENCIES(python-dist copy_python_dist)
