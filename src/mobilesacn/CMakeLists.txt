set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    ${PROJECT_SOURCE_DIR}/resources/resources.qrc
    main.cpp
    log_files.cpp
    LogViewer.cpp
    MainWindow.cpp
    NetIntModel.cpp
    QrCode.cpp
    )

find_package(qr-code-generator REQUIRED)
include(${PROJECT_SOURCE_DIR}/cmake/qsettingscontainer.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE
    libmobilesacn
    qr-code-generator::qr-code-generator
    Qt${Qt_VERSION}::Help
    Qt${Qt_VERSION}::Svg
    Qt${Qt_VERSION}::Widgets
    QSettingsContainer
    )
if (Qt_VERSION EQUAL 6)
    find_package(Qt${Qt_VERSION} COMPONENTS SvgWidgets REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${Qt_VERSION}::SvgWidgets)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
endif ()

# Sentry
set(SENTRY_DSN "" CACHE STRING "sentry.io DSN; unset to not use Sentry.")
if (SENTRY_DSN)
    include(${PROJECT_SOURCE_DIR}/cmake/sentry.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE sentry)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "SENTRY_DSN=\"${SENTRY_DSN}\"")

    include(GNUInstallDirs)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        install_sentry_deps(
            RUNTIME_DIR "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE_DIR "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}"
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        install_sentry_deps(
            RUNTIME_DIR "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS"
            ARCHIVE_DIR "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS"
            LIBRARY_DIR "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS"
        )

        # For some reason, installing the sentry deps into the bundle also installs them into the root of the generated
        # disk image.  Remove that nonsense.
        install(CODE
            "
                file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\" \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}\")
            "
            )
    endif ()
endif ()

# Updater
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(${PROJECT_SOURCE_DIR}/cmake/winsparkle.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE winsparkle::winsparkle)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(Qt${Qt_VERSION} COMPONENTS DBus REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${Qt_VERSION}::DBus)
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ".")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_ORGANIZATION_DOMAIN}.${PROJECT_NAME}"
        MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
else ()
    install(TARGETS ${PROJECT_NAME} RUNTIME)
endif ()

# Qt packaging
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(${PROJECT_NAME} PRIVATE "/NODEFAULTLIB:library")
    configure_file(mobilesacn.rc ${CMAKE_CURRENT_BINARY_DIR}/mobilesacn.rc)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/mobilesacn.rc)
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/mobilesacn.rc APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/resources/logo.ico)
    # Bundle Qt runtime
    if (BUILD_PACKAGE)
        include("${PROJECT_SOURCE_DIR}/cmake/deployqt.cmake")
        deployqt(TARGET ${PROJECT_NAME})
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(COPY "${PROJECT_SOURCE_DIR}/resources/logo.icns" DESTINATION "${PROJECT_BINARY_DIR}/meta")
    file(RENAME "${PROJECT_BINARY_DIR}/meta/logo.icns" "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
    target_sources(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
    set_source_files_properties("${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    # Install assistant.
    if (Qt_VERSION EQUAL 5)
        find_package(Qt${Qt_VERSION} COMPONENTS Help REQUIRED)
    else ()
        find_package(Qt${Qt_VERSION} COMPONENTS Help Tools REQUIRED)
    endif ()
    set(ASSISTANT_PATH "$<TARGET_FILE_DIR:Qt${Qt_VERSION}::qhelpgenerator>/Assistant.app")
    include(${PROJECT_SOURCE_DIR}/cmake/find_qt_tool.cmake)
    find_qt_tool(macdeployqt REQUIRED)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${ASSISTANT_PATH}/" \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/Assistant.app\"
        COMMAND "${macdeployqt_PROG}" \"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/Assistant.app\"
        )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/${PROJECT_HELP_PATH}"
        COMMAND "${CMAKE_COMMAND}" -E copy
        "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qhc"
        "${PROJECT_BINARY_DIR}/doc/qthelp/${PROJECT_NAME}.qch"
        "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/${PROJECT_HELP_PATH}"
        )
    if (BUILD_PACKAGE)
        include("${PROJECT_SOURCE_DIR}/cmake/deployqt.cmake")
        deployqt(TARGET ${PROJECT_NAME})
    endif ()
endif ()

add_dependencies(${PROJECT_NAME} webui_build)
add_dependencies(${PROJECT_NAME} doc_webui)
add_dependencies(${PROJECT_NAME} doc_qthelp_qhc)

add_subdirectory(updater)
