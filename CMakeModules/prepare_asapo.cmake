function(prepare_asapo)
    get_target_property(RECEIVER_DIR receiver-bin BINARY_DIR)
    get_target_property(RECEIVER_NAME receiver-bin OUTPUT_NAME)
    get_target_property(DISCOVERY_FULLPATH asapo-discovery EXENAME)
    get_target_property(AUTHORIZER_FULLPATH asapo-authorizer EXENAME)
    get_target_property(BROKER_FULLPATH asapo-broker EXENAME)
    set(WORK_DIR ${CMAKE_CURRENT_BINARY_DIR})
    if (WIN32)
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json.tpl.win receiver.json.tpl COPYONLY)
    else()
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json.tpl.lin receiver.json.tpl COPYONLY)
    endif()
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/receiver.nmd.in  receiver.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/discovery.nmd.in  discovery.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/authorizer.nmd.in  authorizer.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/broker.nmd.in  broker.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/discovery_settings.json.tpl discovery.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/authorizer_settings.json.tpl authorizer.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json.tpl broker.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_secret.key broker_secret.key COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/nginx.conf.tpl nginx.conf.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/nginx.nmd.in nginx.nmd @ONLY)

endfunction()

macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        string(REGEX REPLACE "\\.in$" "" File ${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${File}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)
