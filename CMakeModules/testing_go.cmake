if (BUILD_TESTS)
    enable_testing()
endif ()

if (BUILD_TESTS)
    set(HIDRA2_MINIMUM_COVERAGE 90)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS
            "--trace-children=yes --leak-check=full --error-exitcode=1 --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.suppressions")
endif ()

function(gotest target test_source_files)
    if (BUILD_TESTS)
                add_test(NAME test-${target} COMMAND go test ${test_source_files}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        set_property(
                TEST
                test-${target}
                PROPERTY
                ENVIRONMENT GOPATH=${gopath})
        message(STATUS "Added test 'test-${target}'")
        add_test(NAME coveragetest-${target}
                        COMMAND ${CMAKE_MODULE_PATH}/coverage_go.sh
                        ${CMAKE_CURRENT_BINARY_DIR} ${HIDRA2_MINIMUM_COVERAGE} ${gopath}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    endif ()
endfunction()

