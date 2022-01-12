set(TARGET_NAME transfer-single-file_kafka)
if (NOT WIN32)
    set(SOURCE_FILES kafka_mock.cpp)

    add_executable(${TARGET_NAME} ${SOURCE_FILES})
    target_link_libraries(${TARGET_NAME} rdkafka)

    #use expression generator to get rid of VS adding Debug/Release folders
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
            )

    ################################
    # Testing
    ################################
    prepare_asapo()

    add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
endif()