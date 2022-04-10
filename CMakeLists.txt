cmake_minimum_required(VERSION 3.19)
project(mobile_sacn
    VERSION 0.0.1
    DESCRIPTION "Mobile sACN test tool"
    HOMEPAGE_URL "https://dankeenan.org"
    )
set(PROJECT_DISPLAY_NAME "Mobile sACN")
set(PROJECT_AUTHOR "Dan Keenan")
set(PROJECT_ORGANIZATION_NAME "dankeenan")
set(PROJECT_ORGANIZATION_DOMAIN "dankeenan.org")
set(SACN_CID "96559a9b-0e40-5417-bf1a-c5b4e8c69ca2" CACHE STRING "sACN CID")

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
set(PROJECT_WEBUI_PATH "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/web")

add_subdirectory(mobile_sacn_webui)
add_subdirectory(proto)
add_subdirectory(src)

set(BUILD_PACKAGE Off CACHE BOOL "Create packages, installers, etc.")
if (BUILD_PACKAGE)
    include(cmake/install.cmake)
endif ()
