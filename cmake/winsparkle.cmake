# Download Github release.
set(WINSPARKLE_VERSION 0.7.0 CACHE STRING "WinSparkle library version")
set(WINSPARKLE_ZIP "${PROJECT_BINARY_DIR}/winsparkle.zip")
set(WINSPARKLE_EXTRACT_DIR "${PROJECT_BINARY_DIR}/_deps/winsparkle")
set(WINSPARKLE_DIR "${WINSPARKLE_EXTRACT_DIR}/WinSparkle-${WINSPARKLE_VERSION}")
if (CMAKE_SIZEOF_VOID_P GREATER 4)
    # 64-bit.
    set(WINSPARKLE_BINARY_DIR "${WINSPARKLE_DIR}/x64/Release")
else ()
    set(WINSPARKLE_BINARY_DIR "${WINSPARKLE_DIR}/Release")
endif ()
if (NOT IS_DIRECTORY "${WINSPARKLE_EXTRACT_DIR}")
    file(DOWNLOAD
        "https://github.com/vslavik/winsparkle/releases/download/v${WINSPARKLE_VERSION}/WinSparkle-${WINSPARKLE_VERSION}.zip"
        "${WINSPARKLE_ZIP}"
        SHOW_PROGRESS
        TLS_VERIFY ON
        INACTIVITY_TIMEOUT 60
        )
    file(ARCHIVE_EXTRACT
        INPUT "${WINSPARKLE_ZIP}"
        DESTINATION "${WINSPARKLE_EXTRACT_DIR}"
        )
endif ()
if (NOT TARGET winsparkle::winsparkle)
    add_library(winsparkle::winsparkle SHARED IMPORTED)
    set_target_properties(winsparkle::winsparkle PROPERTIES
        IMPORTED_LOCATION "${WINSPARKLE_BINARY_DIR}/WinSparkle.dll"
        IMPORTED_IMPLIB "${WINSPARKLE_BINARY_DIR}/WinSparkle.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${WINSPARKLE_DIR}/include"
        )
endif ()
install(FILES "${WINSPARKLE_BINARY_DIR}/WinSparkle.dll" TYPE BIN)
