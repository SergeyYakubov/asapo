configure_file(setup.py.in setup.py @ONLY)
configure_file(MANIFEST.in MANIFEST.in @ONLY)


ADD_CUSTOM_TARGET(python-dist ALL
        COMMAND python setup.py sdist
        )

get_property(ASAPO_WORKER_LIB TARGET asapo-worker PROPERTY LOCATION)



ADD_CUSTOM_TARGET(copy_python_dist ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_wrappers.h ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_worker.cpp ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/worker/api/cpp/include ${CMAKE_CURRENT_BINARY_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASAPO_CXX_COMMON_INCLUDE_DIR}/common ${CMAKE_CURRENT_BINARY_DIR}/include/common
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/include/common/version.h.in
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy ${ASAPO_WORKER_LIB} ${CMAKE_CURRENT_BINARY_DIR}/lib

        )


ADD_DEPENDENCIES(copy_python_dist python-lib3)

ADD_DEPENDENCIES(python-dist copy_python_dist)
