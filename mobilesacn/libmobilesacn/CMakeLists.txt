add_library(libmobilesacn STATIC
    rpc/ChanCheck.h
    rpc/ChanCheck.cpp
    rpc/ReceiveLevels.h
    rpc/ReceiveLevels.cpp
    rpc/RpcHandler.h
    rpc/RpcHandler.cpp
    rpc/SubscribableMergeReceiver.h
    rpc/SubscribableMergeReceiver.cpp
    rpc/SubscribableNotifyHandler.h
    rpc/SubscribableSourceDetector.h
    rpc/SubscribableSourceDetector.cpp
    rpc/TransmitHandler.h
    rpc/TransmitHandler.cpp
    rpc/TransmitLevels.h
    rpc/TransmitLevels.cpp
    Application.h
    Application.cpp
    Caffeine.cpp
    Caffeine.h
    CrowLogHandler.h
    CrowLogHandler.cpp
    EtcPalLogHandler.h
    EtcPalLogHandler.cpp
    Exception.h
    HttpServer.h
    HttpServer.cpp
    SacnCidGenerator.h
    SacnCidGenerator.cpp
    SacnSettings.h
    util.h
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(libmobilesacn PRIVATE
            Caffeine_Linux.cpp
    )
    target_link_libraries(libmobilesacn PRIVATE
            Qt::DBus
    )
else ()
    target_sources(libmobilesacn PRIVATE
            Caffeine_Other.cpp
    )
endif ()

include(FetchContent)
FetchContent_Declare(sACN
    GIT_REPOSITORY "https://github.com/ETCLabs/sACN.git"
    GIT_TAG "v3.1.0.2"
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(sACN)

#if  (CMAKE_SYSTEM_NAME STREQUAL "Linux")
#    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
#endif ()
find_package(Boost COMPONENTS signals2 REQUIRED)
find_package(Crow REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(libmobilesacn PRIVATE
    Crow::Crow
    fmt::fmt
    Qt::Core
    # So we can get QApplication helpers not available in QCoreApplication.
    Qt::Widgets
    ZLIB::ZLIB
    mobile_sacn_messages
    sACN
    )
target_link_libraries(libmobilesacn PUBLIC
    Boost::signals2
    spdlog::spdlog
    EtcPal
)
if  (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
endif ()
target_compile_definitions(libmobilesacn PUBLIC CROW_ENFORCE_WS_SPEC CROW_ENABLE_COMPRESSION)
