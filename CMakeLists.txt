cmake_minimum_required(VERSION 3.28)
project(mobile_sacn
    VERSION 0.3.0
    DESCRIPTION "Mobile sACN test tool"
    HOMEPAGE_URL "https://mobilesacn.dankeenan.org"
    )
set(PROJECT_DISPLAY_NAME "Mobile sACN")
set(PROJECT_AUTHOR "Dan Keenan")
set(PROJECT_ORGANIZATION_NAME "dankeenan")
set(PROJECT_ORGANIZATION_DOMAIN "dankeenan.org")
string(TIMESTAMP PROJECT_BUILD_TIMESTAMP "%s" UTC)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" log -n1 --format=%H
    OUTPUT_VARIABLE PROJECT_COMMIT_SHA
    COMMAND_ERROR_IS_FATAL ANY
)
string(STRIP "${PROJECT_COMMIT_SHA}" PROJECT_COMMIT_SHA)
execute_process(COMMAND "${GIT_EXECUTABLE}" tag --points-at=HEAD
    OUTPUT_VARIABLE PROJECT_COMMIT_TAG
    COMMAND_ERROR_IS_FATAL ANY
)
string(STRIP "${PROJECT_COMMIT_TAG}" PROJECT_COMMIT_TAG)

include(GNUInstallDirs)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PROJECT_WEBUI_PATH "Resources/web")
else ()
    set(PROJECT_WEBUI_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/web")
endif ()

option(BUILD_EXEC "Build the executable program.  You probably want to do this." ON)
add_subdirectory(mobile_sacn_webui)

find_package(Python3 COMPONENTS Interpreter)
option(BUILD_DOC "Build documentation (Requires Python)" ${Python3_FOUND})
if (BUILD_DOC)
    add_subdirectory(doc)
endif ()

if (BUILD_EXEC)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    add_compile_definitions(QT_NO_KEYWORDS)

    include_directories(${PROJECT_SOURCE_DIR}/include)

    # This is more portable across compilers compared to other methods
    message(STATUS "Detected system as ${CMAKE_SYSTEM_NAME}")
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_compile_definitions(PLATFORM_LINUX)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        add_compile_definitions(PLATFORM_WINDOWS)
        add_compile_definitions(NOMINMAX)
        add_compile_definitions(_WIN32_WINNT=0x0601)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_compile_definitions(PLATFORM_MACOS)
    endif ()

    # Decide which Qt version to use.
    find_package(Qt6 COMPONENTS
        Core
        Widgets
        Svg
        SvgWidgets
        REQUIRED
    )
    qt_standard_project_setup()

    add_subdirectory(messages)
    add_subdirectory(src)

    include(CTest)
    if (BUILD_TESTING)
#        add_subdirectory(test)
    endif ()

    option(BUILD_PACKAGE "Create packages, installers, etc." Off)
    if (BUILD_PACKAGE)
        include(cmake/install.cmake)
    endif ()
endif ()
