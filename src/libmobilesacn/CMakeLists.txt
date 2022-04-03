add_library(libmobilesacn
    Application.cpp
    CrowLogHandler.cpp
    EtcPalLogHandler.cpp
    HttpServer.cpp
    sacn.cpp
    )

include(${PROJECT_SOURCE_DIR}/cmake/sacn.cmake)
find_package(Boost COMPONENTS filesystem headers REQUIRED)
find_package(crow REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(libmobilesacn PRIVATE
    mobilesacnproto
    Boost::filesystem
    Boost::headers
    crow::crow
    fmt::fmt
    sACN
    spdlog::spdlog
    )
target_link_libraries(libmobilesacn PUBLIC EtcPal spdlog::spdlog)

add_subdirectory(rpc)
