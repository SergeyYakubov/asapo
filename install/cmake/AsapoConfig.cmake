include(CMakeFindDependencyMacro)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Modules/" ${CMAKE_MODULE_PATH})

find_dependency(CURL REQUIRED)
if(CURL_FOUND) #old FindCURL versions do not create CURL::libcurl target, so we do it here if CURL::libcurl is missing
    if(NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl UNKNOWN IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
        set_target_properties(CURL::libcurl PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${CURL_LIBRARIES}")
    endif()
endif()


find_dependency(Threads REQUIRED)

set(_supported_components Consumer Producer)

set(_supported_versions static shared)
set(static NO)
set(shared NO)
foreach(_comp ${Asapo_FIND_COMPONENTS})
    if (_comp IN_LIST _supported_versions)
        set(${_comp} YES)
    endif ()
endforeach ()

list(REMOVE_ITEM Asapo_FIND_COMPONENTS ${_supported_versions})

if (static AND shared)
    set(Asapo_NOT_FOUND_MESSAGE "Asapo `static` and `shared` components are mutually exclusive.")
    set(Asapo_FOUND FALSE)
    return()
endif ()

if (NOT DEFINED ASAPO_STATIC_CXX_LIBS)
    set(ASAPO_STATIC_CXX_LIBS OFF CACHE BOOL "link with static gcc and stdc++")
endif()


macro(asapo_load_targets type comp)
    if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/Asapo${comp}${type}Target.cmake")
        set(Asapo_NOT_FOUND_MESSAGE
        "Asapo `${comp}/${type}` libraries were requested but not found.")
        set(Asapo_FOUND FALSE)
        return()
    endif ()
    include("${CMAKE_CURRENT_LIST_DIR}/Asapo${_comp}${type}Target.cmake")
    string(TOLOWER ${_comp} comp_low)
    string(TOLOWER ${type} type_low)
    message(STATUS "Added imported::asapo-${comp_low} target (${type_low})")
    if (${type} STREQUAL "Static" AND DEFINED ASAPO_STATIC_CXX_LIBS AND ASAPO_STATIC_CXX_LIBS AND CMAKE_COMPILER_IS_GNUCXX)
        if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
            message(FATAL_ERROR "Need at least CMake 3.13 to add target link options for static gcc libs, use SET(CMAKE_EXE_LINKER_FLAGS  \"\${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++\") instead ")
        else()
            message(STATUS "Added linker options -static-libgcc -static-libstdc++ to imported::asapo-${comp_low}")
            target_link_options(imported::asapo-${comp_low} INTERFACE -static-libgcc -static-libstdc++)
        endif()

    endif()

endmacro()

macro(asapo_load_comp_targets comp static shared)
    if (static)
        asapo_load_targets(Static ${comp})
    elseif (shared)
        asapo_load_targets(Shared ${comp})
    elseif (DEFINED ASAPO_SHARED_LIBS AND ASAPO_SHARED_LIBS)
        asapo_load_targets(Shared ${comp})
    elseif (DEFINED ASAPO_SHARED_LIBS AND NOT ASAPO_SHARED_LIBS)
        asapo_load_targets(Static ${comp})
    elseif (BUILD_SHARED_LIBS)
        if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/Asapo${comp}SharedTarget.cmake")
            asapo_load_targets(Shared ${comp})
        else ()
            asapo_load_targets(Static ${comp})
        endif ()
    else ()
        if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/Asapo${comp}StaticTarget.cmake")
            asapo_load_targets(Static ${comp})
        else ()
            asapo_load_targets(Shared ${comp})
        endif ()
    endif ()
endmacro()


if( "S${Asapo_FIND_COMPONENTS}" STREQUAL "S")
    foreach(_comp ${_supported_components})
        include("${CMAKE_CURRENT_LIST_DIR}/Asapo${_comp}StaticTarget.cmake")
    endforeach()
else()
    foreach(_comp ${Asapo_FIND_COMPONENTS})
        if (NOT _comp IN_LIST _supported_components)
            set(Asapo_FOUND False)
            set(Asapo_NOT_FOUND_MESSAGE "Unsupported component: ${_comp}")
        endif()
        asapo_load_comp_targets(${_comp} ${static} ${shared})
    endforeach()
endif()

