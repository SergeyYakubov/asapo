string(TIMESTAMP TIMESTAMP "%H:%M:%S %d.%m.%Y UTC" UTC)

configure_file( ${PROJECT_SOURCE_DIR}/common/cpp/include/asapo/common/internal/version.h.in ../common/cpp/include/asapo/common/internal/version.h @ONLY)
configure_file( ${PROJECT_SOURCE_DIR}/common/go/src/asapo_common/version/version_lib.go.in ../common/go/src/asapo_common/version/version_lib.go @ONLY)
