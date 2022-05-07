add_library(libmobilesacn
    Application.cpp
    CrowLogHandler.cpp
    EtcPalLogHandler.cpp
    HttpServer.cpp
    sacn.cpp
    SacnMerger.cpp
    )

include(${PROJECT_SOURCE_DIR}/cmake/sacn.cmake)
find_package(Boost COMPONENTS filesystem headers REQUIRED)
#find_package(crow REQUIRED)
# Use forked crow for a bugfix until a new release is made.
include(FetchContent)
FetchContent_Declare(crow
    GIT_REPOSITORY "https://github.com/CrowCpp/Crow.git"
    GIT_TAG "master"
    )
FetchContent_GetProperties(crow)
if (NOT crow_POPULATED)
    FetchContent_Populate(crow)
    add_subdirectory(${crow_SOURCE_DIR} ${crow_BINARY_DIR} EXCLUDE_FROM_ALL)
    # Match conan's naming convention.
    add_library(crow::crow ALIAS Crow)
endif ()
if  (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
endif ()
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(libmobilesacn PRIVATE
    mobilesacnproto
    Boost::filesystem
    Boost::headers
    crow::crow
    fmt::fmt
    sACN
    )
target_link_libraries(libmobilesacn PUBLIC EtcPal spdlog::spdlog)

add_subdirectory(fx)
add_subdirectory(rpc)
