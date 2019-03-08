execute_process(COMMAND git describe --tags --dirty OUTPUT_VARIABLE VERSION)
string(STRIP ${VERSION} VERSION)


execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE BRANCH)
string(STRIP ${BRANCH} BRANCH)

if (${BRANCH} STREQUAL "develop")
    SET (ASAPO_VERSION ${BRANCH}.${VERSION})
else()
    SET (ASAPO_VERSION ${BRANCH}.latest)
endif()

string(TIMESTAMP TIMESTAMP "%H:%M:%S %d.%m.%Y UTC" UTC)

configure_file(${PROJECT_SOURCE_DIR}/common/cpp/include/common/version.h.in ${PROJECT_SOURCE_DIR}/common/cpp/include/common/version.h @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/common/go/src/asapo_common/version/version_lib.go.in ${PROJECT_SOURCE_DIR}/common/go/src/asapo_common/version/version_lib.go @ONLY)
