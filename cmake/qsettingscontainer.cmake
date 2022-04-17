include(FetchContent)
FetchContent_Declare(qsettingscontainer
    GIT_REPOSITORY "https://github.com/danielskeenan/QSettingsContainer.git"
    GIT_TAG "v1.0.0"
    )
FetchContent_GetProperties(qsettingscontainer)
if (NOT qsettingscontainer_POPULATED)
    FetchContent_Populate(qsettingscontainer)
    add_subdirectory(${qsettingscontainer_SOURCE_DIR} ${qsettingscontainer_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
