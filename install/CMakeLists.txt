set(CPACK_PACKAGE_NAME "${PROJECT_DISPLAY_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_AUTHOR}")
set(CPACK_PACKAGE_CONTACT "${PROJECT_AUTHOR} <dk@dankeenan.org>")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_DISPLAY_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_MONOLITHIC_INSTALL On)
set(CPACK_STRIP_FILES On)

# Build the license files as HTML, RTF, and plain text
find_program(PANDOC_PROG NAMES pandoc)
if (PANDOC_PROG)
    set(LICENSE_SOURCE_FILE "${PROJECT_SOURCE_DIR}/LICENSE.md")
    add_custom_target(licenses
            COMMAND ${PANDOC_PROG}
            "--embed-resources"
            "--standalone"
            "--from" "commonmark"
            "--to" "html"
            "--eol" "lf"
            "--output" "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.html"
            "${LICENSE_SOURCE_FILE}"
            COMMAND ${PANDOC_PROG}
            "--standalone"
            "--embed-resources"
            "--standalone"
            "--from" "commonmark"
            "--to" "rtf"
            "--eol" "lf"
            "--output" "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf"
            "${LICENSE_SOURCE_FILE}"
            SOURCES "${LICENSE_SOURCE_FILE}"
    )
endif ()

set(PLATFORM_INSTALLER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}")
if (EXISTS "${PLATFORM_INSTALLER_DIR}/CMakeLists.txt")
    add_subdirectory("${PLATFORM_INSTALLER_DIR}")
endif ()
include(CPack)
