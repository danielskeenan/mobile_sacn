add_library(libmobilesacn STATIC
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/ChanCheck.h
    rpc/ChanCheck.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/RpcHandler.h
    rpc/RpcHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/Application.h
    Application.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/EtcPalLogHandler.h
    EtcPalLogHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/Exception.h
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/HttpServer.h
    HttpServer.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/sacn.h
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/SacnMerger.h
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
    mobile_sacn_messages
    sACN
    )
target_link_libraries(libmobilesacn PUBLIC spdlog::spdlog EtcPal)
