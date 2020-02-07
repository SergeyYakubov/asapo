find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
    file(MAKE_DIRECTORY html)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    message("Found doxygen - ${DOXYGEN_EXECUTABLE}.")
    add_custom_target(
            docs-cpp-producer
            COMMAND
            PROJECT_NUMBER=${ASAPO_VERSION} ${DOXYGEN_EXECUTABLE} doxygen.ini VERBATIM
            WORKING_DIRECTORY producer
    )
    add_custom_target(
            docs-cpp-consumer
            COMMAND
            PROJECT_NUMBER=${ASAPO_VERSION} ${DOXYGEN_EXECUTABLE} doxygen.ini VERBATIM
            WORKING_DIRECTORY consumer
    )
    add_custom_target(
            docs-cpp
            COMMAND
            PROJECT_NUMBER=${ASAPO_VERSION}  ${DOXYGEN_EXECUTABLE} doxygen.ini VERBATIM
    )
    add_dependencies(docs-cpp docs-cpp-producer docs-cpp-consumer)


else()
    message(FATAL_ERROR "Unable to find doxygen.")
endif()
