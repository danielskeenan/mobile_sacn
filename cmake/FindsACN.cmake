set(sACN_VERSION "3.1.0.2")

include(FetchContent)
FetchContent_Declare(sACN
        GIT_REPOSITORY "https://github.com/ETCLabs/sACN.git"
        GIT_TAG "v${sACN_VERSION}"
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(sACN)

if (TARGET sACN AND NOT TARGET sACN::sACN)
    add_library(sACN::sACN ALIAS sACN)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sACN
        REQUIRED_VARS
        sacn_POPULATED
        VERSION_VAR sACN_VERSION
)
