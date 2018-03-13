set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/receiver.h src/receiver.cpp
        src/network_producer_peer.h src/network_producer_peer.cpp src/network_producer_peer_handlers.cpp
        )


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})

add_executable(${TARGET_NAME}-bin src/main.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME ${TARGET_NAME})
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME}-bin ${HIDRA2_COMMON_IO_LIBRARIES})
set_target_properties(${TARGET_NAME}-bin PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)
#
set(TEST_SOURCE_FILES
        unittests/test_receiver.cpp
        unittests/test_network_producer_peer.cpp
        )
#
set(TEST_LIBRARIES "${TARGET_NAME};system_io")
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
