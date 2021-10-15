cmake_minimum_required(VERSION 3.5)

PROJECT(ASAPO_HA)

add_subdirectory (Packages)

set (CONSUL_VERSION 1.10.3)
set (NOMAD_VERSION 1.1.6)
set (TERRAFORM_VERSION 1.0.8)

INCLUDE(ExternalProject)
ExternalProject_Add(
        consul
        URL https://releases.hashicorp.com/consul/${CONSUL_VERSION}/consul_${CONSUL_VERSION}_linux_amd64.zip
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        CMAKE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

ExternalProject_Add(
        nomad
        URL https://releases.hashicorp.com/nomad/${NOMAD_VERSION}/nomad_${NOMAD_VERSION}_linux_amd64.zip
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        CMAKE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

ExternalProject_Add(
        terraform
        URL https://releases.hashicorp.com/terraform/${TERRAFORM_VERSION}/terraform_${TERRAFORM_VERSION}_linux_amd64.zip
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
ExternalProject_Get_Property(terraform SOURCE_DIR)
set (TERRAFORM_SOURCE_DIR ${SOURCE_DIR})



install(PROGRAMS ${CONSUL_SOURCE_DIR}/consul DESTINATION bin)
install(PROGRAMS ${NOMAD_SOURCE_DIR}/nomad DESTINATION bin)

install(PROGRAMS ${TERRAFORM_SOURCE_DIR}/terraform DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/usr/lib/systemd/system/
        DESTINATION  /usr/lib/systemd/system)