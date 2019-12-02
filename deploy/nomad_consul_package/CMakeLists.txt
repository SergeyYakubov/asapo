cmake_minimum_required(VERSION 3.5)

PROJECT(ASAPO_HA)

add_subdirectory (Packages)

INCLUDE(ExternalProject)
ExternalProject_Add(
        consul
        URL https://releases.hashicorp.com/consul/1.2.0/consul_1.2.0_linux_amd64.zip
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        CMAKE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

ExternalProject_Add(
        nomad
        URL https://releases.hashicorp.com/nomad/0.8.4/nomad_0.8.4_linux_amd64.zip
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        CMAKE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)


ExternalProject_Get_Property(consul SOURCE_DIR)
set (CONSUL_SOURCE_DIR ${SOURCE_DIR})
ExternalProject_Get_Property(nomad SOURCE_DIR)
set (NOMAD_SOURCE_DIR ${SOURCE_DIR})

install(PROGRAMS ${CONSUL_SOURCE_DIR}/consul DESTINATION bin)
install(PROGRAMS ${NOMAD_SOURCE_DIR}/nomad DESTINATION bin)

install(PROGRAMS ${NOMAD_SOURCE_DIR}/nomad DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/usr/lib/systemd/system/
        DESTINATION  /usr/lib/systemd/system)