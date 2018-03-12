set(TARGET_NAME producer-api)
set(SOURCE_FILES
        src/producer.cpp
        src/producer_impl.h src/producer_impl.cpp
        )


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:system_io>)
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################
set(TEST_SOURCE_FILES
        unittests/test_producer_impl.cpp
        unittests/test_producer.cpp
        )
set(TEST_LIBRARIES "${TARGET_NAME}")


gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
