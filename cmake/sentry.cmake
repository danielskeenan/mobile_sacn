include(FetchContent)
FetchContent_Declare(
    sentry_native
    GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
    GIT_TAG 0.4.17
)

set(SENTRY_BUILD_SHARED_LIBS Off CACHE BOOL "" FORCE)
FetchContent_GetProperties(sentry_native)
if (NOT sentry_native_POPULATED)
    FetchContent_Populate(sentry_native)
endif ()
# Sentry has a number of install() commands we don't want.
add_subdirectory("${sentry_native_SOURCE_DIR}" "${sentry_native_BINARY_DIR}" EXCLUDE_FROM_ALL)
target_compile_definitions(sentry INTERFACE SENTRY_BUILD_STATIC=1)

# This is a function to allow it to be called near where the target is defined.  This is required for installing
# inside a Mac OS App Bundle.
function(install_sentry_deps)
    cmake_parse_arguments(SENTRY_DEPS "" "TARGET;RUNTIME_DIR;ARCHIVE_DIR;LIBRARY_DIR" "" ${ARGN})
    set(REQUIRED_ARGS TARGET RUNTIME_DIR ARCHIVE_DIR LIBRARY_DIR)
    foreach (REQUIRED_ARG IN ITEMS ${REQUIRED_ARGS})
        if (NOT SENTRY_DEPS_${REQUIRED_ARG})
            message(FATAL_ERROR "install_sentry_deps ${REQUIRED_ARG} must be specified. (Got ${SENTRY_DEPS_${REQUIRED_ARG}})")
        endif ()
    endforeach ()

    include(GNUInstallDirs)
    set(_OUTPUT_TYPES
        RUNTIME
        ARCHIVE
        LIBRARY
        )
    set(_CRASHPAD_LIBS
        crashpad_client
        crashpad_compat
        crashpad_getopt
        crashpad_handler
        crashpad_handler_lib
        crashpad_minidump
        crashpad_snapshot
        crashpad_tools
        crashpad_util
        crashpad_zlib
        mini_chromium
        )
    foreach (_TARGET IN ITEMS ${_CRASHPAD_LIBS})
        foreach (_OUTPUT_TYPE IN ITEMS ${_OUTPUT_TYPES})
            if (NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
                install(TARGETS ${_TARGET} ${_OUTPUT_TYPE} DESTINATION "${SENTRY_DEPS_${_OUTPUT_TYPE}_DIR}")
            else ()
                add_custom_command(TARGET ${SENTRY_DEPS_TARGET} POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E make_directory \"${SENTRY_DEPS_${_OUTPUT_TYPE}_DIR}\"
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    \"$<TARGET_PROPERTY:${_TARGET},${_OUTPUT_TYPE}_OUTPUT_DIRECTORY>/$<TARGET_PROPERTY:${_TARGET},${_OUTPUT_TYPE}_OUTPUT_NAME>\"
                    \"${SENTRY_DEPS_${_OUTPUT_TYPE}_DIR}\"
                    )
            endif ()
        endforeach ()
    endforeach ()
endfunction()
