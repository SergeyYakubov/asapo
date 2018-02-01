set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/main.cpp
        src/receiver.h src/receiver.cpp
        src/network_producer_peer.h src/network_producer_peer.cpp src/network_producer_peer_handlers.cpp
        )


################################
# Library
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:common>)
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${HIDRA2_COMMON_IO_LIBRARIES})

set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)


################################
# Testing
################################

#set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)
#
#set(TEST_SOURCE_FILES
#        unittests/test_receiver.cpp
#        )
#
#set(TEST_LIBRARIES "${TARGET_NAME} common")
#gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

