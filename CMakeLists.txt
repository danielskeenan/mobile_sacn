cmake_minimum_required(VERSION 3.28)
project(mobile_sacn
    VERSION 0.5.1
    DESCRIPTION "Mobile sACN test tool"
    HOMEPAGE_URL "https://mobilesacn.dankeenan.org"
    )
set(PROJECT_DISPLAY_NAME "Mobile sACN")
set(PROJECT_AUTHOR "Dan Keenan")
set(PROJECT_ORGANIZATION_NAME "dankeenan")
set(PROJECT_ORGANIZATION_DOMAIN "dankeenan.org")
string(TIMESTAMP PROJECT_BUILD_TIMESTAMP "%s" UTC)
set(PROJECT_MSI_UPGRADE_CODE "A41ADB0F-BFDC-56C2-971C-517ACC3EB249")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

find_package(Python3 COMPONENTS Interpreter)
option(BUILD_DOC "Build documentation (Requires Python)" ${Python3_FOUND})
if (BUILD_DOC)
    add_subdirectory(doc)
endif ()

if (BUILD_EXEC)
    add_compile_definitions(QT_NO_KEYWORDS)

    include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

    # This is more portable across compilers compared to other methods
    string(TOUPPER "${CMAKE_SYSTEM_NAME}" OS_DEF)
    add_compile_definitions("OS_${OS_DEF}")
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        add_compile_definitions(NOMINMAX)
        # Target Windows 10 (https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt)
        add_compile_definitions("WINVER=0x0A00" "_WIN32_WINNT=0x0A00")
    endif ()

    # Decide which Qt version to use.
    set(QT_COMPONENTS
            Core
            Network
            Svg
            WebSockets
            Widgets
            Xml
    )
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        list(APPEND QT_COMPONENTS "DBus")
    endif ()
    find_package(Qt6 COMPONENTS ${QT_COMPONENTS} REQUIRED)

    qt_standard_project_setup()

    # Log levels
    add_compile_definitions("SPDLOG_ACTIVE_LEVEL=$<IF:$<CONFIG:Debug>,0,2>")

    add_subdirectory(mobile_sacn_webui)
    add_subdirectory(messages)
    add_subdirectory(mobilesacn)

    include(CTest)
    if (BUILD_TESTING)
#        add_subdirectory(test)
    endif ()

    option(BUILD_PACKAGE "Create packages, installers, etc." Off)
    if (BUILD_PACKAGE)
        include(cmake/install.cmake)
    endif ()
endif ()
