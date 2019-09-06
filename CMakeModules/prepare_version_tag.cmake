execute_process(COMMAND git describe --tags --dirty
                OUTPUT_VARIABLE VERSION
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${VERSION} VERSION)

execute_process(COMMAND git rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE BRANCH
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${BRANCH} BRANCH)

if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION ${BRANCH}.${VERSION})
else()
    SET (ASAPO_VERSION ${BRANCH}.latest)
endif()

execute_process(COMMAND git describe --tags --abbrev=0
                OUTPUT_VARIABLE VERSION_TAGS
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${VERSION_TAGS} VERSION_TAGS)

execute_process(COMMAND git rev-parse --short HEAD
                OUTPUT_VARIABLE VERSION_COMMIT
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
string(STRIP ${VERSION_COMMIT} VERSION_COMMIT)

function(cleanup varname)
    string (REPLACE "-" "_" out ${${varname}})
    SET( ${varname} ${out} PARENT_SCOPE)
endfunction()

cleanup(BRANCH)


if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION_PYTHON ${VERSION_TAGS}.${BRANCH}.${VERSION_COMMIT})
else()
    SET (ASAPO_VERSION_PYTHON ${VERSION_TAGS}.${BRANCH}.latest)
endif()


message("Asapo Version: " ${ASAPO_VERSION})
message("Asapo Version Python: " ${ASAPO_VERSION_PYTHON})
