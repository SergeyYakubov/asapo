execute_process(COMMAND git describe --tags --dirty OUTPUT_VARIABLE VERSION)
string(STRIP ${VERSION} VERSION)

execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE BRANCH)
string(STRIP ${BRANCH} BRANCH)

if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION ${BRANCH}.${VERSION})
else()
    SET (ASAPO_VERSION ${BRANCH}.latest)
endif()

