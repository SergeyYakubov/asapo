if (BUILD_TESTS)
    enable_testing()
    set(HIDRA2_MINIMUM_COVERAGE 70)
    find_package(Threads)
endif ()

function(gtest target test_source_files test_libraries)
    if (BUILD_TESTS)
        include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
        add_executable(test-${target} ${test_source_files})
        target_link_libraries(test-${target} gtest gtest_main ${CMAKE_THREAD_LIBS_INIT})
        if (NOT ${test_libraries} STREQUAL "")
            target_link_libraries(test-${target} ${test_libraries})
        endif ()
        add_test(NAME test-${target} COMMAND test-${target})

        message(STATUS "Added test 'test-${target}'")

        if ((CMAKE_COMPILER_IS_GNUCXX) OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang"))
            include(CodeCoverage)
            APPEND_COVERAGE_COMPILER_FLAGS()
            set (COVERAGE_EXCLUDES '*/unittests/*')
            SETUP_TARGET_FOR_COVERAGE(NAME coverage-${target} EXECUTABLE test-${target} ${target})
            add_test(NAME coveragetest-${target}
                    COMMAND ${CMAKE_MODULE_PATH}/check_test.sh
                    coverage-${target} ${CMAKE_BINARY_DIR} ${HIDRA2_MINIMUM_COVERAGE}
                    )
            SET_TESTS_PROPERTIES(coveragetest-${target} PROPERTIES DEPENDS test-${target})
            set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
            set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
        endif ()
    endif ()
endfunction()
