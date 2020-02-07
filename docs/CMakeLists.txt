if(BUILD_CPP_DOCS)
    add_subdirectory(doxygen)
endif()

if(BUILD_PYTHON_DOCS)
    add_subdirectory (sphinx)
endif()

