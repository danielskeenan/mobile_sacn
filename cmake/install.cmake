set(CPACK_PACKAGE_VENDOR "${PROJECT_AUTHOR}")
set(CPACK_PACKAGE_CONTACT "${PROJECT_AUTHOR} <dk@dankeenan.org>")
#set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/app-icon.svg")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
include("${CMAKE_CURRENT_LIST_DIR}/license.cmake")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.html")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_DISPLAY_NAME}")
set(CPACK_MONOLITHIC_INSTALL On)
set(CPACK_STRIP_FILES On)

# Linux metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(GNUInstallDirs)
    # .desktop file is filled in with configure-time info
    configure_file("${PROJECT_SOURCE_DIR}/meta/linux/${PROJECT_NAME}.desktop" "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.desktop")
    install(FILES "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

    # App icon
#    install(FILES "${PROJECT_SOURCE_DIR}/resources/app-icon.svg"
#        RENAME "${PROJECT_NAME}.svg"
#        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps")

    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS On)
    set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
endif ()

# Windows metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.rtf")
    set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_DISPLAY_NAME}")
    set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_DISPLAY_NAME}")
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
endif ()

# Mac OS metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.rtf")
endif ()

include(CPack)
