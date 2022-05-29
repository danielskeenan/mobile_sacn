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
    Qt${Qt_VERSION}::Widgets
    Qt${Qt_VERSION}::Svg
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
        include("${PROJECT_SOURCE_DIR}/cmake/Windeployqt.cmake")
        windeployqt(${PROJECT_NAME} bin)
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(COPY "${PROJECT_SOURCE_DIR}/resources/logo.icns" DESTINATION "${PROJECT_BINARY_DIR}/meta")
    file(RENAME "${PROJECT_BINARY_DIR}/meta/logo.icns" "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
    target_sources(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
    set_source_files_properties("${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    if (BUILD_PACKAGE)
        include("${PROJECT_SOURCE_DIR}/cmake/Macdeployqt.cmake")
        macdeployqt(${PROJECT_NAME} bin)
    endif ()
endif ()

add_dependencies(${PROJECT_NAME} webui_build)

add_subdirectory(updater)
