set(TARGET_NAME asapo-fabric)

include_directories(include)

set(SOURCE_FILES asapo_fabric.cpp)

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
ELSE()
	set(SOURCE_FILES ${SOURCE_FILES}
			fabric_factory_not_supported.cpp
		)
ENDIF()

################################
# Library
################################

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>)

target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
