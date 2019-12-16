if(DEFINED ENV{INSTALLPATH_NOMAD_JOBS})
    SET (NOMAD_INSTALL $ENV{INSTALLPATH_NOMAD_JOBS}/nomad_jobs)
else()
    SET (NOMAD_INSTALL ${CMAKE_INSTALL_PREFIX}/nomad_jobs)
endif()

configure_files(${CMAKE_CURRENT_SOURCE_DIR}/asapo_services ${CMAKE_CURRENT_BINARY_DIR}/asapo_services)
configure_files(${CMAKE_CURRENT_SOURCE_DIR}/asapo_services/scripts ${CMAKE_CURRENT_BINARY_DIR}/asapo_services/scripts)




