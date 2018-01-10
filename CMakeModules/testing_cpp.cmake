if (BUILD_TESTS OR BUILD_INTEGRATION_TESTS OR BUILD_EXAMPLES)
    enable_testing()
endif ()

if (BUILD_TESTS)
    set(HIDRA2_MINIMUM_COVERAGE 70)
    find_package(Threads)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS
            "--trace-children=yes --leak-check=full --error-exitcode=1 --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.suppressions")
    if (NOT "$ENV{gtest_SOURCE_DIR}" STREQUAL "")
        set(gtest_SOURCE_DIR $ENV{gtest_SOURCE_DIR})
    endif ()
    message(STATUS "Will look for google test at ${gtest_SOURCE_DIR}")
endif ()

function(gtest target test_source_files test_libraries)
    if (BUILD_TESTS)
        include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
        link_directories(${gtest_SOURCE_DIR}/lib)
        add_executable(test-${target} ${test_source_files})

        IF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            set(GTEST_LIBS gtestd gtest_maind gmockd)
        ELSE ()
            set(GTEST_LIBS gtest gmock gtest_main)
        ENDIF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_link_libraries(test-${target} ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})

        if (NOT ${test_libraries} STREQUAL "")
            target_link_libraries(test-${target} ${test_libraries})
        endif ()
        add_test(NAME test-${target} COMMAND test-${target})
        set_tests_properties(test-${target} PROPERTIES LABELS "unit;all")

        message(STATUS "Added test 'test-${target}'")

        if (CMAKE_COMPILER_IS_GNUCXX)
            include(CodeCoverage)
            APPEND_COVERAGE_COMPILER_FLAGS()
            set(COVERAGE_EXCLUDES '*/unittests/*')
            SETUP_TARGET_FOR_COVERAGE(NAME coverage-${target} EXECUTABLE test-${target} ${target})
            add_test(NAME coveragetest-${target}
                    COMMAND ${CMAKE_MODULE_PATH}/check_test.sh
                    coverage-${target} ${CMAKE_BINARY_DIR} ${HIDRA2_MINIMUM_COVERAGE})
            set_tests_properties(coveragetest-${target} PROPERTIES LABELS "coverage;all")
            SET_TESTS_PROPERTIES(coveragetest-${target} PROPERTIES DEPENDS test-${target})
            set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
            set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
        endif ()

        add_memory_test(${target} test-${target} "" "" "unit")

    endif ()
endfunction()

function(add_memory_test target executable commandargs fixture label)
    if (BUILD_TESTS OR BUILD_INTEGRATION_TESTS)
        if (MEMORYCHECK_COMMAND)
            set(memcheck_args ${MEMORYCHECK_COMMAND_OPTIONS})
            separate_arguments(memcheck_args)
            set(args ${commandargs})
            separate_arguments(args)
            add_test(NAME memcheck-${target} COMMAND ${MEMORYCHECK_COMMAND} ${memcheck_args}
                    ${CMAKE_CURRENT_BINARY_DIR}/${executable} ${args})
            set_tests_properties(memcheck-${target} PROPERTIES
                    LABELS "memcheck_${label};all"
                    DEPENDS test-${target}
                    )
            if (NOT ${fixture} STREQUAL "")
                set_tests_properties(memcheck-${target} PROPERTIES
                        FIXTURES_REQUIRED ${fixture}
                        )
            endif ()
        endif ()
    endif ()
endfunction()

function(add_test_setup_cleanup exename)
    if (BUILD_INTEGRATION_TESTS)
        IF (WIN32)
            add_test(NAME test-${exename}-setup COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/setup_windows.bat)
            add_test(NAME test-${exename}-cleanup COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_windows.bat)
        ELSE()
            add_test(NAME test-${exename}-setup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/setup_linux.sh)
            add_test(NAME test-${exename}-cleanup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_linux.sh)
        ENDIF()
        set_tests_properties(test-${exename}-setup PROPERTIES FIXTURES_SETUP test-${exename}-fixture)
        set_tests_properties(test-${exename}-cleanup PROPERTIES FIXTURES_CLEANUP test-${exename}-fixture)
    endif ()
endfunction()

function(add_integration_test exename testname commandargs)
    if (BUILD_INTEGRATION_TESTS)
        set(args ${commandargs})
        separate_arguments(args)
        add_test(NAME test-${exename}-${testname} COMMAND ${exename} ${args})
        set_tests_properties(test-${exename}-${testname} PROPERTIES
                LABELS "integration;all"
                FIXTURES_REQUIRED test-${exename}-fixture
                )
        if (ARGN)
            set(commandargs ${ARGN})
        endif ()
        if (NOT "${ARGN}" STREQUAL nomem)
            message(STATUS "memory check ${exename}")
            add_memory_test(${exename}-${testname} ${exename}
                    "${commandargs}" test-${exename}-fixture
                    "integration")
        endif ()
    endif ()
endfunction()

function(add_script_test testname exename)
    if (BUILD_EXAMPLES)
        set(args ${exename})
        separate_arguments(args)
        IF (WIN32)
            add_test(NAME test-${testname} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check_windows.bat
                      ${args})
        ELSE()
            set(memargs ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ${exename})
            separate_arguments(memargs)
            add_test(NAME test-${testname} COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/check_linux.sh
                      ${args})
            add_test(NAME memtest-${testname} COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/check_linux.sh
                    ${memargs})
        ENDIF()
        set_tests_properties(test-${testname} PROPERTIES
                LABELS "example;all"
                )
    endif ()
endfunction()
