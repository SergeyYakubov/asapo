set(TARGET_NAME version)
set(SOURCE_FILES
        version.cpp
)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/3d_party/rapidjson/include)
