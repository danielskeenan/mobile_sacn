# TODO: Refactor this out into a subdirectory per-platform (like Patchman).
set(CPACK_PACKAGE_NAME "${PROJECT_DISPLAY_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_AUTHOR}")
set(CPACK_PACKAGE_CONTACT "${PROJECT_AUTHOR} <dk@dankeenan.org>")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
include("${CMAKE_CURRENT_LIST_DIR}/license.cmake")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.html")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_DISPLAY_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_MONOLITHIC_INSTALL On)
set(CPACK_STRIP_FILES On)

# Linux metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    include(GNUInstallDirs)
    # .desktop file is filled in with configure-time info
    configure_file("${PROJECT_SOURCE_DIR}/meta/linux/${PROJECT_NAME}.desktop" "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.desktop")
    install(FILES "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

    # App icon
    install(FILES "${PROJECT_SOURCE_DIR}/resources/logo.svg"
        RENAME "${PROJECT_NAME}.svg"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps")

    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS On)
    set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
    set(CPACK_DEBIAN_PACKAGE_NAME "mobilesacn")
    set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-only")
endif ()

# Windows metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_GENERATOR "WIX;ZIP")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.rtf")
    set(CPACK_WIX_UPGRADE_GUID "A41ADB0F-BFDC-56C2-971C-517ACC3EB249")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER ".")
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/logo.ico")
    set(CPACK_WIX_PRODUCT_ICON "${PROJECT_SOURCE_DIR}/resources/logo.ico")
    include(InstallRequiredSystemLibraries)
endif ()

# Mac OS metadata
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.rtf")
endif ()

include(CPack)
