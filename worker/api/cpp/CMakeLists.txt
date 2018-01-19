set(TARGET_NAME hidra2-worker)

set(SOURCE_FILES include/worker/data_broker.h src/data_broker.cpp src/folder_data_broker.cpp src/folder_data_broker.h include/hidra2_worker.h)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:common>)

target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)
#target_link_libraries(${TARGET_NAME} $<TARGET_OBJECTS:common>)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_worker_api.cpp
                      unittests/test_folder_broker.cpp)
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

install(TARGETS ${TARGET_NAME} DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)
