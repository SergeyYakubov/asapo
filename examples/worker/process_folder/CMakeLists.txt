set(TARGET_NAME worker_processfodler)
set(SOURCE_FILES process_folder.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} common worker-api pthread)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
