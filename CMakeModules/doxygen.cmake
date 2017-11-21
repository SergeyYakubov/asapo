find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
    message("Found doxygen. Using doxygen to build documentaion.")
    add_custom_target(
            documentation
            COMMAND
            ${DOXYGEN_EXECUTABLE} doxygen.ini VERBATIM
            WORKING_DIRECTORY ..
    )
else()
    message("Unable to find doxygen. Skipping code documentaion.")
endif()
