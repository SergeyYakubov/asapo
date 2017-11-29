function(astyle target source_files)
    find_program(ASTYLE_EXECUTABLE astyle)
    if(ASTYLE_EXECUTABLE)
        message("Found astyle, using astyle to format code of target ${target}.")
        add_custom_command(
                TARGET ${target} PRE_BUILD
                COMMAND
                ${ASTYLE_EXECUTABLE} -n --style=1tbs --indent-namespaces --indent-preproc-block ${source_files}
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
    else()
        message("Unable to find astyle. Skipping code formatting for ${target}")
    endif()
endfunction()
