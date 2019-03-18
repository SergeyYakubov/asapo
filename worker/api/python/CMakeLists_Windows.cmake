configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

find_package (Python3 REQUIRED COMPONENTS Development)
find_package (Numpy REQUIRED)
message ("   Python libaries:" ${Python3_LIBRARIES})
message ("   Python includes:" ${Python3_INCLUDE_DIRS})
message ("   Numpy:" ${PYTHON_NUMPY_INCLUDE_DIR})


add_custom_command(OUTPUT asapo_worker.cpp
        COMMAND ${Python3_EXECUTABLE} cythonize.py
        DEPENDS asapo-worker)


set(TARGET_NAME asapo_worker)

set(SOURCE_FILES
        asapo_worker.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".pyd")

target_link_libraries(${TARGET_NAME}  asapo-worker ${Python3_LIBRARIES})
target_include_directories(${TARGET_NAME} PUBLIC include  ${Python3_INCLUDE_DIRS} ${PYTHON_NUMPY_INCLUDE_DIR})


add_subdirectory(binary_dist_windows)
