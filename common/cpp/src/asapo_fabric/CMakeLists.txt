set(TARGET_NAME asapo-fabric)

include_directories(include)

set(SOURCE_FILES asapo_fabric.cpp)

IF(ENABLE_LIBFABRIC)
	set(SOURCE_FILES ${SOURCE_FILES}
		fabric_factory_impl.cpp
		)
ELSE()
	set(SOURCE_FILES ${SOURCE_FILES}
		fabric_factory_not_supported.cpp
		)
ENDIF()

################################
# Library
################################

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
