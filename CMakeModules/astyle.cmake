find_program(ASTYLE_EXECUTABLE astyle)

add_custom_target(ASTYLE)

if(ASTYLE_EXECUTABLE)
	message(STATUS "Found astyle, using astyle to format code of target ${target}.")
	add_custom_target(astyle ALL
		COMMAND
		${ASTYLE_EXECUTABLE} -i
			--exclude=${PROJECT_BINARY_DIR}
			--recursive -n --style=google --indent=spaces=4
			"${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.h"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		VERBATIM	
        )
else()
	message(WARNING "Unable to find astyle. Code formatting willbe skipped")
endif()

