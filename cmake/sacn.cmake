include_guard(GLOBAL)

include(FetchContent)
FetchContent_Declare(
    sacn
    GIT_REPOSITORY "https://github.com/ETCLabs/sACN.git"
    GIT_TAG "v2.0.1"
)
FetchContent_GetProperties(sacn)
if (NOT sacn_POPULATED)
    FetchContent_Populate(sacn)
    add_subdirectory(${sacn_SOURCE_DIR} ${sacn_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
