include(FetchContent)
FetchContent_Declare(qsettingscontainer
    GIT_REPOSITORY "https://github.com/danielskeenan/QSettingsContainer.git"
    GIT_TAG "v1.0.0"
    EXCLUDE_FROM_ALL
    )
FetchContent_MakeAvailable(qsettingscontainer)
