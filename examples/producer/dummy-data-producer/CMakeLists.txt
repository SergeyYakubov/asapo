set(TARGET_NAME dummy-data-producer)
set(SOURCE_FILES
        dummy_data_producer.cpp
        )

add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:common>)
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${HIDRA2_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} producer-api)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()
