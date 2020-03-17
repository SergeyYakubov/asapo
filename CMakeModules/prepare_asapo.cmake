function(prepare_asapo)
    get_target_property(RECEIVER_DIR receiver-bin BINARY_DIR)
    get_target_property(RECEIVER_NAME receiver-bin OUTPUT_NAME)
    get_target_property(DISCOVERY_FULLPATH asapo-discovery EXENAME)
    get_target_property(AUTHORIZER_FULLPATH asapo-authorizer EXENAME)
    get_target_property(FILE_TRANSFER_FULLPATH asapo-file-transfer EXENAME)
    get_target_property(BROKER_FULLPATH asapo-broker EXENAME)
    set(WORK_DIR ${CMAKE_CURRENT_BINARY_DIR})

    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/asap3 ASAP3_FOLDER )
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/beamline CURRENT_BEAMLINES_FOLDER )

    if (WIN32)
        string(REPLACE "\\" "\\\\" ASAP3_FOLDER "${ASAP3_FOLDER}")
        string(REPLACE "\\" "\\\\" CURRENT_BEAMLINES_FOLDER "${CURRENT_BEAMLINES_FOLDER}")
    endif()

    set (ASAP3_FOLDER "${ASAP3_FOLDER}" PARENT_SCOPE)
    set (CURRENT_BEAMLINES_FOLDER "${CURRENT_BEAMLINES_FOLDER}" PARENT_SCOPE)

    if(NOT DEFINED RECEIVER_USE_CACHE)
        set(RECEIVER_USE_CACHE true)
    endif()

    if(NOT DEFINED RECEIVER_WRITE_TO_DISK)
        set(RECEIVER_WRITE_TO_DISK true)
    endif()

    if (WIN32)
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json.tpl.win.in receiver.json.tpl @ONLY)
        configure_file(${CMAKE_SOURCE_DIR}/config/nomad/nginx_kill_win.nmd nginx_kill.nmd @ONLY)
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/authorizer_settings.json.tpl.win authorizer.json.tpl COPYONLY)
    else()
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json.tpl.lin.in receiver.json.tpl @ONLY)
        configure_file(${CMAKE_SOURCE_DIR}/config/nomad/nginx_kill_lin.nmd nginx_kill.nmd @ONLY)
        configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/authorizer_settings.json.tpl.lin authorizer.json.tpl COPYONLY)
    endif()

    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/receiver.nmd.in  receiver.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/discovery.nmd.in  discovery.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/authorizer.nmd.in  authorizer.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/file_transfer.nmd.in  file_transfer.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/config/nomad/broker.nmd.in  broker.nmd @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/discovery_settings.json.tpl discovery.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json.tpl broker.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/file_transfer_settings.json.tpl file_transfer.json.tpl COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/auth_secret.key auth_secret.key COPYONLY)
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
