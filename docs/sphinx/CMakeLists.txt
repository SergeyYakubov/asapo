file(COPY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


add_custom_target(
        docs-py ALL
        COMMAND
        make html VERBATIM
        WORKING_DIRECTORY sphinx
)

add_dependencies(docs-py python-lib-producer)
add_dependencies(docs-py python-lib)
