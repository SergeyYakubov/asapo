if (BUILD_TESTS)
    enable_testing()
endif ()

if (BUILD_TESTS)
    set(ASAPO_MINIMUM_COVERAGE 80)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS
            "--trace-children=yes --leak-check=full --error-exitcode=1 --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.suppressions")
endif ()

function(gotest target source_dir test_source_files)
    if (BUILD_TESTS)
                add_test(NAME test-${target} COMMAND go test ${test_source_files}
                WORKING_DIRECTORY ${source_dir})
        set_tests_properties(test-${target} PROPERTIES LABELS "unit;all")
        message(STATUS "Added test 'test-${target}'")
        if (CMAKE_COMPILER_IS_GNUCXX)
        add_test(NAME coveragetest-${target}
                        COMMAND ${PROJECT_SOURCE_DIR}/CMakeModules/coverage_go.sh
                        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ASAPO_MINIMUM_COVERAGE} ${gopath}
                        WORKING_DIRECTORY ${source_dir})
        set_tests_properties(coveragetest-${target} PROPERTIES LABELS "coverage;all")
        endif()
    endif ()
endfunction()

function(go_integration_test target source_dir test_source_files label)
    if (BUILD_TESTS)
        add_test(NAME test-${target} COMMAND go test ${test_source_files} -run ${label}
                -tags integration_tests
                WORKING_DIRECTORY ${source_dir})
        message(STATUS "Added test 'test-${target}'")
    endif ()
endfunction()
