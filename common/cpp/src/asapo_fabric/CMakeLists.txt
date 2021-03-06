set(TARGET_NAME asapo-fabric)

include_directories(include)

set(SOURCE_FILES
		asapo_fabric.cpp
		fabric_factory_not_supported.cpp
	)

IF(ENABLE_LIBFABRIC)
	set(SOURCE_FILES ${SOURCE_FILES}
            fabric_internal_error.cpp
			fabric_factory_impl.cpp
			common/fabric_context_impl.cpp
			common/fabric_memory_region_impl.cpp
            common/task/fabric_waitable_task.cpp
			common/task/fabric_self_deleting_task.cpp
			common/task/fabric_self_requeuing_task.cpp
			common/task/fabric_alive_check_response_task.cpp
			client/fabric_client_impl.cpp
			server/fabric_server_impl.cpp
            server/task/fabric_recv_any_task.cpp
			server/task/fabric_handshake_accepting_task.cpp
		)
ENDIF()

################################
# Library
################################

add_library(asapo_fabric_objects OBJECT ${SOURCE_FILES})
target_include_directories(asapo_fabric_objects PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR} )
target_include_directories(asapo_fabric_objects SYSTEM PUBLIC ${LIBFABRIC_INCLUDE_DIR})

add_library(${TARGET_NAME} STATIC $<TARGET_OBJECTS:asapo_fabric_objects> $<TARGET_OBJECTS:system_io>)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR} )
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${LIBFABRIC_INCLUDE_DIR})
