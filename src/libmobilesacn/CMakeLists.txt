add_library(libmobilesacn STATIC
    Application.cpp
    EtcPalLogHandler.cpp
    HttpServer.cpp
)

include(FetchContent)
FetchContent_Declare(sACN
    GIT_REPOSITORY "https://github.com/ETCLabs/sACN.git"
    GIT_TAG "v3.1.0.2"
)
FetchContent_MakeAvailable(sACN)

#if  (CMAKE_SYSTEM_NAME STREQUAL "Linux")
#    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
#endif ()
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(libmobilesacn PRIVATE
    fmt::fmt
    Qt::Core
    Qt::HttpServer
    Qt::WebSockets
    sACN
    )
target_link_libraries(libmobilesacn PUBLIC spdlog::spdlog EtcPal)
