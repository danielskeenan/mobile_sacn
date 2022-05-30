cmake_minimum_required(VERSION 3.19)
project(mobile_sacn
    VERSION 0.1.1
    DESCRIPTION "Mobile sACN test tool"
    HOMEPAGE_URL "https://mobilesacn.dankeenan.org"
    )
set(PROJECT_DISPLAY_NAME "Mobile sACN")
set(PROJECT_AUTHOR "Dan Keenan")
set(PROJECT_ORGANIZATION_NAME "dankeenan")
set(PROJECT_ORGANIZATION_DOMAIN "dankeenan.org")
string(TIMESTAMP PROJECT_BUILD_TIMESTAMP "%s" UTC)
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" log -n1 --format=%H
    OUTPUT_VARIABLE PROJECT_COMMIT_SHA
    COMMAND_ERROR_IS_FATAL ANY
    )
string(STRIP "${PROJECT_COMMIT_SHA}" PROJECT_COMMIT_SHA)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/proto)

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
include(GNUInstallDirs)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PROJECT_WEBUI_PATH "Resources/web")
    set(PROJECT_HELP_PATH "Resources/help")
else ()
    set(PROJECT_WEBUI_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/web")
    set(PROJECT_HELP_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/help")
endif ()

option(BUILD_EXEC "Build the executable program.  You probably want to do this." ON)
if (BUILD_EXEC)
    # Allow testing.
    include(CTest)

    # Decide which Qt version to use.
    set(Qt_COMPONENTS Widgets Svg Help)
    find_package(Qt6 COMPONENTS ${Qt_COMPONENTS})
    if (NOT Qt6_FOUND)
        find_package(Qt5 COMPONENTS ${Qt_COMPONENTS})
        set(Qt_VERSION 5)
    else ()
        set(Qt_VERSION 6)
    endif ()

    add_subdirectory(mobile_sacn_webui)
    add_subdirectory(proto)
    add_subdirectory(src)

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif ()

    option(BUILD_PACKAGE "Create packages, installers, etc." Off)
    if (BUILD_PACKAGE)
        include(cmake/install.cmake)
    endif ()
endif ()

option(BUILD_DOC "Build user documentation." On)
if (BUILD_DOC)
    add_subdirectory(doc)
endif ()
