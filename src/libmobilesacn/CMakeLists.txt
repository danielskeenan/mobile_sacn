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
find_package(crow REQUIRED)
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
