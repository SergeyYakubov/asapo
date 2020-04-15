set(TARGET_NAME beamtime_metadata)
set(SOURCE_FILES
        beamtime_metadata.cpp
        )

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
#GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES} asapo-producer)


#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
#        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
#)


#if (CMAKE_COMPILER_IS_GNUCXX)
#    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
#endif()


add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")
