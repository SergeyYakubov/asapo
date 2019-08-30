file(COPY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


add_custom_target(
        pydoc ALL
        COMMAND
        make html VERBATIM
        WORKING_DIRECTORY sphinx
)

add_dependencies(pydoc python-lib2-producer)
add_dependencies(pydoc python-lib2)
