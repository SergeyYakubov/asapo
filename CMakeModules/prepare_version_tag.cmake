execute_process(COMMAND git describe --tags --dirty OUTPUT_VARIABLE VERSION)
string(STRIP ${VERSION} VERSION)

execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE BRANCH)
string(STRIP ${BRANCH} BRANCH)

if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION ${BRANCH}.${VERSION})
else()
    SET (ASAPO_VERSION ${BRANCH}.latest)
endif()

execute_process(COMMAND git describe --tags --abbrev=0 OUTPUT_VARIABLE VERSION_TAGS)
string(STRIP ${VERSION_TAGS} VERSION_TAGS)

execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE VERSION_COMMIT)
string(STRIP ${VERSION_COMMIT} VERSION_COMMIT)

function(cleanup varname)
    string (REPLACE "-" "_" out ${${varname}})
    SET( ${varname} ${out} PARENT_SCOPE)
endfunction()

cleanup(BRANCH)

if (${BRANCH} STREQUAL "master")
    SET (ASAPO_VERSION_DOCKER_SUFFIX "")
else()
    SET (ASAPO_VERSION_DOCKER_SUFFIX "-dev")
endif()



if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION_PYTHON ${VERSION_TAGS}.${BRANCH}.${VERSION_COMMIT})
else()
    SET (ASAPO_VERSION_PYTHON ${VERSION_TAGS}.${BRANCH}.latest)
endif()


message("Asapo Version: " ${ASAPO_VERSION})
message("Asapo Version Python: " ${ASAPO_VERSION_PYTHON})