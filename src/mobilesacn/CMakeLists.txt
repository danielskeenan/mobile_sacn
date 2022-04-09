set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}
    main.cpp
    LogViewer.cpp
    MainWindow.cpp
    NetIntModel.cpp
    )

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    libmobilesacn
    Qt::Widgets
    )

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ".")
else ()
    install(TARGETS ${PROJECT_NAME} RUNTIME)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_ORGANIZATION_DOMAIN}.${PROJECT_NAME}"
#    MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns
    MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

# Qt packaging
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(${PROJECT_NAME} PRIVATE "/NODEFAULTLIB:library")
    # Bundle Qt runtime
    if (BUILD_PACKAGE)
        include("${PROJECT_SOURCE_DIR}/cmake/Windeployqt.cmake")
        windeployqt(${PROJECT_NAME} bin)
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#    file(COPY "${PROJECT_SOURCE_DIR}/resources/app-icon.icns" DESTINATION "${PROJECT_BINARY_DIR}/meta")
#    file(RENAME "${PROJECT_BINARY_DIR}/meta/app-icon.icns" "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
#    target_sources(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns")
#    set_source_files_properties("${PROJECT_BINARY_DIR}/meta/${PROJECT_NAME}.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    if (BUILD_PACKAGE)
        include("${PROJECT_SOURCE_DIR}/cmake/Macdeployqt.cmake")
        macdeployqt(${PROJECT_NAME} bin)
    endif ()
endif ()

#add_dependencies(${PROJECT_NAME} webui_build)
