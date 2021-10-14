file(COPY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)

add_custom_target(
        site ALL
        COMMAND
        npm run build VERBATIM
)

add_custom_target(
        freeze_version
        COMMAND
        ./freeze_version.sh
)
