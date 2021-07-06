configure_file(setup.py.in setup.py @ONLY)
configure_file(MANIFEST.in MANIFEST.in @ONLY)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

if ("rpm" IN_LIST BUILD_PYTHON_PACKAGES)
	ADD_CUSTOM_TARGET(python-rpm-producer ALL
		COMMAND PACKAGE_PREFIX=python- python setup.py bdist_rpm --release=1.${PACKAGE_RELEASE_SUFFIX}
			--requires=numpy --binary-only
			COMMAND rm -f dist/python*.gz dist/*debuginfo* dist/*debugsource*
		)

	ADD_CUSTOM_TARGET(python3-rpm-producer ALL
		COMMAND PACKAGE_PREFIX=python3- python3 setup.py bdist_rpm --release=1.${PACKAGE_RELEASE_SUFFIX}
			--requires=python3-numpy --binary-only
		COMMAND rm -f dist/python3*.gz dist/*debuginfo* dist/*debugsource*
		)
	ADD_DEPENDENCIES(python3-rpm-producer python-rpm-producer)
	ADD_DEPENDENCIES(python-rpm-producer copy_python_dist-producer)
endif()

if ("deb" IN_LIST BUILD_PYTHON_PACKAGES)
	ADD_CUSTOM_TARGET(python-deb-producer ALL
			COMMAND rm -rf deb_dist/*/
			COMMAND PACKAGE_PREFIX= python setup.py --command-packages=stdeb.command
			sdist_dsc --debian-version=${PACKAGE_RELEASE_SUFFIX} --depends=python-numpy bdist_deb
			COMMAND rm -f deb_dist/*dbgsym*
			COMMAND cp deb_dist/*.deb dist/
			)

	ADD_CUSTOM_TARGET(python3-deb-producer ALL
			COMMAND rm -rf deb_dist/*/
			COMMAND PACKAGE_PREFIX= python3 setup.py --command-packages=stdeb.command
			sdist_dsc --debian-version=${PACKAGE_RELEASE_SUFFIX} --depends3=python3-numpy bdist_deb
			COMMAND rm -f deb_dist/*dbgsym*
			COMMAND cp deb_dist/*.deb dist/
			)
	ADD_DEPENDENCIES(python3-deb-producer python-deb-producer)
	ADD_DEPENDENCIES(python-deb-producer copy_python_dist-producer)
endif()

if ("source" IN_LIST BUILD_PYTHON_PACKAGES )
	ADD_CUSTOM_TARGET(python-dist-producer ALL
        COMMAND ${Python_EXECUTABLE} setup.py sdist
        )
	ADD_DEPENDENCIES(python-dist-producer copy_python_dist-producer)
endif()

ADD_CUSTOM_TARGET(copy_python_dist-producer ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_wrappers.h ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../asapo_producer.cpp ${CMAKE_CURRENT_BINARY_DIR}/.
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/producer/api/cpp/include ${CMAKE_CURRENT_BINARY_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/common ${CMAKE_CURRENT_BINARY_DIR}/include/asapo/common
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/preprocessor ${CMAKE_CURRENT_BINARY_DIR}/include/asapo/preprocessor
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/include/common/version.h.in
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/logger ${CMAKE_CURRENT_BINARY_DIR}/include/asapo/logger
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:asapo-producer> ${CMAKE_CURRENT_BINARY_DIR}/lib
        )

if (UNIX)
	ADD_DEPENDENCIES(copy_python_dist-producer python-lib-producer)
ELSE()
	ADD_DEPENDENCIES(copy_python_dist-producer asapo_producer)
ENDIF()

