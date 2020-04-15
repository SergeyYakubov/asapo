if (BUILD_TESTS OR BUILD_INTEGRATION_TESTS OR BUILD_EXAMPLES)
    enable_testing()
endif ()

if (BUILD_TESTS)
    set(ASAPO_MINIMUM_COVERAGE 70)
    find_package(Threads)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS
            "--trace-children=yes --leak-check=full --error-exitcode=1 --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.suppressions")
    if (NOT "$ENV{gtest_SOURCE_DIR}" STREQUAL "")
        set(gtest_SOURCE_DIR $ENV{gtest_SOURCE_DIR})
    endif ()
    message(STATUS "Will look for google test at ${gtest_SOURCE_DIR}")
    if (CMAKE_COMPILER_IS_GNUCXX)
        include(CodeCoverage)
        APPEND_COVERAGE_COMPILER_FLAGS()
    endif ()
endif ()

#TODO: Call add_plain_unit_test in gtest
function(add_plain_unit_test target test_source_files linktarget)
    if (BUILD_TESTS)
        include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
        link_directories(${gtest_SOURCE_DIR}/lib)

        add_executable(test-${target} ${test_source_files})

        if (NOT ${libs} STREQUAL "")
            target_link_libraries(test-${target} ${libs})
        endif ()

        IF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            set(GTEST_LIBS gtestd gtest_maind gmockd)
        ELSE ()
            set(GTEST_LIBS gtest gmock gtest_main)
        ENDIF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_link_libraries(test-${target} ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})

        GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
        message(STATUS "ASAPO_COMMON_IO_LIBRARIES: '${ASAPO_COMMON_IO_LIBRARIES}'")
        target_link_libraries(test-${target} ${ASAPO_COMMON_IO_LIBRARIES})

        if (NOT ${test_libraries} STREQUAL "")
            target_link_libraries(test-${target} ${test_libraries})
        endif ()
        add_test(NAME test-${target} COMMAND test-${target})
        set_tests_properties(test-${target} PROPERTIES LABELS "unit;all")

        message(STATUS "Added test 'test-${target}'")
    endif()
endfunction()

function(gtest target test_source_files linktarget)
    if (BUILD_TESTS)
        include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
        link_directories(${gtest_SOURCE_DIR}/lib)

        FOREACH (lib ${linktarget})
            if (NOT ${lib} STREQUAL "")
                get_target_property(target_type ${lib} TYPE)
                if (target_type STREQUAL "OBJECT_LIBRARY")
                    list(APPEND OBJECT "$<TARGET_OBJECTS:${lib}>")
                else ()
                    list(APPEND libs "${lib}")

                endif ()
            endif ()
        ENDFOREACH ()

        add_executable(test-${target} ${test_source_files} ${OBJECT})

        if (NOT ${libs} STREQUAL "")
            target_link_libraries(test-${target} ${libs})
        endif ()

        IF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            set(GTEST_LIBS gtestd gtest_maind gmockd)
        ELSE ()
            set(GTEST_LIBS gtest gmock gtest_main)
        ENDIF (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_link_libraries(test-${target} ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})

        GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
        message(STATUS "ASAPO_COMMON_IO_LIBRARIES: '${ASAPO_COMMON_IO_LIBRARIES}'")
        target_link_libraries(test-${target} ${ASAPO_COMMON_IO_LIBRARIES})

        if (NOT ${test_libraries} STREQUAL "")
            target_link_libraries(test-${target} ${test_libraries})
        endif ()
        add_test(NAME test-${target} COMMAND test-${target})
        set_tests_properties(test-${target} PROPERTIES LABELS "unit;all")

        message(STATUS "Added test 'test-${target}'")

        if (CMAKE_COMPILER_IS_GNUCXX)
            set(COVERAGE_EXCLUDES "*/unittests/*" "*/3d_party/*" "*/python/*")
            if (ARGN)
                set(COVERAGE_EXCLUDES ${COVERAGE_EXCLUDES} ${ARGN})
            endif ()
            SETUP_TARGET_FOR_COVERAGE(NAME coverage-${target} EXECUTABLE test-${target} ${target})
            add_test(NAME coveragetest-${target}
                    COMMAND ${CMAKE_MODULE_PATH}/check_test.sh
                    coverage-${target} ${CMAKE_BINARY_DIR} ${ASAPO_MINIMUM_COVERAGE})
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

function(add_test_setup exename)
    if (BUILD_INTEGRATION_TESTS)
        IF (WIN32)
            add_test(NAME test-${exename}-setup COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/setup_windows.bat)
        ELSE ()
            add_test(NAME test-${exename}-setup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/setup_linux.sh)
        ENDIF ()
        set_tests_properties(test-${exename}-setup PROPERTIES FIXTURES_SETUP test-${exename}-fixture)
    endif ()
endfunction()

function(add_test_cleanup exename)
    if (BUILD_INTEGRATION_TESTS)
        IF (WIN32)
            add_test(NAME test-${exename}-cleanup COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_windows.bat)
        ELSE ()
            add_test(NAME test-${exename}-cleanup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_linux.sh)
        ENDIF ()
        set_tests_properties(test-${exename}-cleanup PROPERTIES FIXTURES_CLEANUP test-${exename}-fixture)
    endif ()
endfunction()

function(add_test_setup_cleanup exename)
    add_test_setup(${exename})
    add_test_cleanup(${exename})
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

function(add_script_test testname arguments)
    if (BUILD_EXAMPLES)
        set(args ${arguments})
        separate_arguments(args)
        IF (WIN32)
            add_test(NAME test-${testname} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check_windows.bat
                    ${args})
        ELSE ()
            add_test(NAME test-${testname} COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/check_linux.sh
                    ${args})
            if (MEMORYCHECK_COMMAND)
                if (ARGN)
                    set(commandargs ${ARGN})
                endif ()
                if (NOT "${ARGN}" STREQUAL "nomem")
                    set(memargs ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ${arguments})
                    separate_arguments(memargs)
                    add_test(NAME memtest-${testname} COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/check_linux.sh
                            ${memargs})
                    set_tests_properties(memtest-${testname} PROPERTIES
                            LABELS "memcheck_${label};all"
                            DEPENDS test-${testname}
                            )

                endif ()
            endif ()
        ENDIF ()
        set_tests_properties(test-${testname} PROPERTIES
                LABELS "example;all"
                )
    endif ()
endfunction()
