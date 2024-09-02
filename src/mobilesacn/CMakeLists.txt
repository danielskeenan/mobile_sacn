add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    ${PROJECT_SOURCE_DIR}/resources/resources.qrc
    main.cpp
    log_files.cpp
    LogViewer.cpp
    MainWindow.cpp
    NetIntModel.cpp
    QrCode.cpp
    )

find_package(unofficial-nayuki-qr-code-generator CONFIG REQUIRED)
include(${PROJECT_SOURCE_DIR}/cmake/qsettingscontainer.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE
    libmobilesacn
    unofficial::nayuki-qr-code-generator::nayuki-qr-code-generator
    Qt::Svg
    Qt::SvgWidgets
    Qt::Widgets
    QSettingsContainer
    )
add_dependencies(${PROJECT_NAME} mobile_sacn_webui ${PROJECT_NAME}_doc)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
endif ()

# Updater
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(WinSparkle REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE WinSparkle::WinSparkle)
    target_sources(${PROJECT_NAME} PRIVATE updater/updater_win.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # TODO: Mac updater
    target_sources(${PROJECT_NAME} PRIVATE updater/updater_none.cpp)
else ()
    target_sources(${PROJECT_NAME} PRIVATE updater/updater_none.cpp)
endif ()


# Installation
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
    TYPE BIN
)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # This is a very ugly kludge because TARGET_RUNTIME_DLLS does not pick up ZLib.
    find_package(ZLIB REQUIRED)
    get_target_property(ZLIB_INC_DIR ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS ${ZLIB_INC_DIR})
    install(
        FILES "${ZLIB_INC_DIR}/../$<$<CONFIG:Debug>:debug/>bin/zlib$<$<CONFIG:Debug>:d>1.dll"
        TYPE BIN
    )
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(Qt6 COMPONENTS DBus REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_ORGANIZATION_DOMAIN}.${PROJECT_NAME}"
        MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
endif ()

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT DEPLOY_SCRIPT
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${DEPLOY_SCRIPT})
endif ()
