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
    add_subdirectory("${sentry_native_SOURCE_DIR}" "${sentry_native_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
target_compile_definitions(sentry INTERFACE SENTRY_BUILD_STATIC=1)
