function(cleanup varname)
    string (REPLACE "-" "_" out ${${varname}})
    SET( ${varname} ${out} PARENT_SCOPE)
endfunction()

execute_process(COMMAND git describe --tags --abbrev=0
                OUTPUT_VARIABLE ASAPO_TAG
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${ASAPO_TAG} ASAPO_TAG)

execute_process(COMMAND git rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE BRANCH
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${BRANCH} BRANCH)
cleanup(BRANCH)

execute_process(COMMAND git rev-parse --short=10 HEAD
        OUTPUT_VARIABLE ASAPO_VERSION_COMMIT
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${ASAPO_VERSION_COMMIT} ASAPO_VERSION_COMMIT)

if (${BRANCH} STREQUAL "master")
    SET (ASAPO_VERSION ${ASAPO_TAG})
    SET (ASAPO_VERSION_COMMIT "")
    SET (ASAPO_VERSION_DOCKER_SUFFIX "")
    SET (PYTHON_ASAPO_VERSION ${ASAPO_VERSION})
else()
    SET (ASAPO_VERSION ${BRANCH})
    SET (ASAPO_VERSION_COMMIT ", build ${ASAPO_VERSION_COMMIT}")
    SET (ASAPO_VERSION_DOCKER_SUFFIX "-dev")
    string(REPLACE "_" "-" ASAPO_VERSION ${ASAPO_VERSION})
    SET (ASAPO_VERSION 1.0.${ASAPO_VERSION})
    SET (PYTHON_ASAPO_VERSION ${ASAPO_VERSION})
endif()

message("Asapo Version: " ${ASAPO_VERSION})
message("Python Asapo Version: " ${PYTHON_ASAPO_VERSION})
message("Asapo commit: " ${ASAPO_VERSION_COMMIT})
