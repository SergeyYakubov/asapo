add_subdirectory(src/system_io)

add_subdirectory(src/common)

add_subdirectory(src/json_parser)

add_subdirectory(src/data_structs)

add_subdirectory(src/version)

add_subdirectory(src/kafka_client)

add_subdirectory(src/http_client)

add_subdirectory(src/logger)

add_subdirectory(src/request)

add_subdirectory(src/asapo_fabric)

if(NOT BUILD_CLIENTS_ONLY)
    add_subdirectory(src/database)
endif()

install(DIRECTORY ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/common
        DESTINATION include/asapo
        PATTERN "*.h.in" EXCLUDE
        PATTERN "*/internal" EXCLUDE)

install(DIRECTORY ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/logger
        DESTINATION include/asapo )

install(DIRECTORY ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/asapo_fabric
        DESTINATION include/asapo)

install(DIRECTORY ${ASAPO_CXX_COMMON_INCLUDE_DIR}/asapo/preprocessor
        DESTINATION include/asapo)
