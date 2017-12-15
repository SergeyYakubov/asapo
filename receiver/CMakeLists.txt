set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/main.cpp
        src/receiver.h src/receiver.cpp
        src/network_producer_peer.h src/network_producer_peer.cpp src/network_producer_peer_handlers.cpp
        src/file_reference_handler.h src/file_reference_handler.cpp
        )


################################
# Library
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
#target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} common pthread)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_receiver.cpp unittests/test_file_reference_handler.cpp src/receiver.h src/receiver.cpp src/network_producer_peer.h src/network_producer_peer.cpp src/network_producer_peer_handlers.cpp src/file_reference_handler.h src/file_reference_handler.cpp)
set(TEST_LIBRARIES common pthread)

gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
