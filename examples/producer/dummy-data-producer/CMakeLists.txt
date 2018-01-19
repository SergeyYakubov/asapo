set(TARGET_NAME dummy-event-detector)
set(SOURCE_FILES
        dummy_data_producer.cpp
        )

add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:common>)
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)
target_link_libraries(${TARGET_NAME} pthread producer-api)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
