configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

find_package (Python3 REQUIRED COMPONENTS Development)
find_package (Numpy REQUIRED)
message ("   Python libaries:" ${Python3_LIBRARIES})
message ("   Python includes:" ${Python3_INCLUDE_DIRS})
message ("   Numpy:" ${PYTHON_NUMPY_INCLUDE_DIR})


add_custom_command(OUTPUT asapo_producer.cpp
        COMMAND ${Python3_EXECUTABLE} cythonize.py
        DEPENDS asapo-producer)


set(TARGET_NAME asapo_producer)

set(SOURCE_FILES
        asapo_producer.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".pyd")
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )

target_link_libraries(${TARGET_NAME}  asapo-producer ${Python3_LIBRARIES})
target_include_directories(${TARGET_NAME} PUBLIC include  ${Python3_INCLUDE_DIRS} ${PYTHON_NUMPY_INCLUDE_DIR})


add_subdirectory(binary_dist_windows)
add_subdirectory(source_dist_linux)
