prepare_asapo()

IF (WIN32)
    configure_file(start_services.bat start_services.bat @ONLY)
    configure_file(stop_services.bat stop_services.bat @ONLY)
ELSE ()
    configure_file(start_services.sh start_services.sh @ONLY)
    configure_file(stop_services.sh stop_services.sh @ONLY)
ENDIF()
