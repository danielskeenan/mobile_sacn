find_path(WinSparkle_INCLUDE_DIR NAMES "winsparkle.h" PATH_SUFFIXES winsparkle)
find_library(WinSparkle_LIBRARY NAMES WinSparkle.lib PATH_SUFFIXES winsparkle)
find_program(WinSparkle_DLL NAMES WinSparkle.dll PATH_SUFFIXES winsparkle)
mark_as_advanced(
        WinSparkle_INCLUDE_DIR
        WinSparkle_LIBRARY
        WinSparkle_DLL
)

# Determine version.
if (WinSparkle_INCLUDE_DIR)
    file(READ "${WinSparkle_INCLUDE_DIR}/winsparkle-version.h" WINSPARKLE_VERSION_H)
    if (WINSPARKLE_VERSION_H MATCHES "#define +WIN_SPARKLE_VERSION_MAJOR +([0-9]+)")
        set(WinSparkle_VERSION_MAJOR ${CMAKE_MATCH_1})
    else ()
        unset(WinSparkle_VERSION_MAJOR)
    endif ()
    if (WINSPARKLE_VERSION_H MATCHES "#define +WIN_SPARKLE_VERSION_MINOR +([0-9]+)")
        set(WinSparkle_VERSION_MINOR ${CMAKE_MATCH_1})
    else ()
        unset(WinSparkle_VERSION_MINOR)
    endif ()
    if (WINSPARKLE_VERSION_H MATCHES "#define +WIN_SPARKLE_VERSION_MICRO +([0-9]+)")
        set(WinSparkle_VERSION_PATCH ${CMAKE_MATCH_1})
    else ()
        unset(WinSparkle_VERSION_PATCH)
    endif ()

    if (DEFINED WinSparkle_VERSION_MAJOR AND DEFINED WinSparkle_VERSION_MINOR AND WinSparkle_VERSION_PATCH)
        set(WinSparkle_VERSION "${WinSparkle_VERSION_MAJOR}.${WinSparkle_VERSION_MINOR}.${WinSparkle_VERSION_PATCH}")
    else ()
        unset(WinSparkle_VERSION)
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WinSparkle
        REQUIRED_VARS
        WinSparkle_DLL
        WinSparkle_LIBRARY
        WinSparkle_INCLUDE_DIR
        VERSION_VAR WinSparkle_VERSION
)
if (WinSparkle_FOUND)
    set(WinSparkle_LIBRARIES ${WinSparkle_LIBRARY})
    set(WinSparkle_INCLUDE_DIRS ${WinSparkle_INCLUDE_DIR})
    if (NOT TARGET WinSparkle::WinSparkle)
        add_library(WinSparkle::WinSparkle SHARED IMPORTED)
        set_target_properties(WinSparkle::WinSparkle PROPERTIES
                IMPORTED_LOCATION "${WinSparkle_DLL}"
                IMPORTED_IMPLIB "${WinSparkle_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${WinSparkle_INCLUDE_DIR}"
        )
    endif ()
endif ()
