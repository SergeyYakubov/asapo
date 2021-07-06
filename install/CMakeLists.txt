include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoConfigVersion.cmake"
        VERSION ${ASAPO_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(cmake/AsapoConfig.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoConfig.cmake"
        COPYONLY
        )

install(
        FILES
        cmake/AsapoConfig.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/asapo/AsapoConfigVersion.cmake"
        DESTINATION
        ${ConfigPackageLocation}
)

install(
        FILES
        ${PROJECT_SOURCE_DIR}/CMakeModules/FindCURL.cmake
        DESTINATION
        ${ConfigPackageLocation}/Modules
)

install(
        FILES
        ${PROJECT_SOURCE_DIR}/CMakeModules/FindPackageHandleStandardArgs.cmake
        DESTINATION
        ${ConfigPackageLocation}/Modules
)

install(
        FILES
        ${PROJECT_SOURCE_DIR}/CMakeModules/FindPackageMessage.cmake
        DESTINATION
        ${ConfigPackageLocation}/Modules
)

install(
        FILES
        ${PROJECT_SOURCE_DIR}/CMakeModules/SelectLibraryConfigurations.cmake
        DESTINATION
        ${ConfigPackageLocation}/Modules
)


