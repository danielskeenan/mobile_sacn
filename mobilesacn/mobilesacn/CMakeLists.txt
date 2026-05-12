qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${PROJECT_SOURCE_DIR}/resources/resources.qrc
        main.cpp
        ClientTableModel.cpp
        ClientTableModel.h
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        NetIntModel.cpp
        NetIntModel.h
        Updater.cpp
        Updater.h
        log_files.cpp
        log_files.h
)

find_package(unofficial-nayuki-qr-code-generator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        libmobilesacn
        unofficial::nayuki-qr-code-generator::nayuki-qr-code-generator
        Qt::Network
        Qt::Svg
        Qt::Widgets
)
if (TARGET ${PROJECT_NAME}_doc)
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_doc)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
    target_sources(${PROJECT_NAME} PRIVATE mobilesacn.rc)
    # Needed for updater.
    target_link_libraries(${PROJECT_NAME} PRIVATE msi.dll)
endif ()

if (BUILD_DOC)
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_doc)
endif ()


# Installation
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
        TYPE BIN
)
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
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_PREFIX=$<TARGET_FILE_DIR:${PROJECT_NAME}>" -DQT_DEPLOY_BIN_DIR=.
            -DQT_DEPLOY_LIB_DIR=. -P "${DEPLOY_SCRIPT}"
            VERBATIM
    )
endif ()
