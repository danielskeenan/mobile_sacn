include(FetchContent)
FetchContent_Declare(
    sentry_native
    GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
    GIT_TAG 0.4.17
)

set(SENTRY_BUILD_SHARED_LIBS Off CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sentry_native)
target_compile_definitions(sentry INTERFACE SENTRY_BUILD_STATIC=1)
