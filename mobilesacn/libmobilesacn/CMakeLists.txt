add_library(libmobilesacn STATIC
        handler/BaseHandler.cpp
        handler/BaseHandler.h
        handler/ChanCheck.cpp
        handler/ChanCheck.h
        handler/MergeReceiver.cpp
        handler/MergeReceiver.h
        handler/ReceiveLevels.cpp
        handler/ReceiveLevels.h
        handler/SourceDetector.cpp
        handler/SourceDetector.h
        handler/TransmitHandler.cpp
        handler/TransmitHandler.h
        handler/TransmitLevels.cpp
        handler/TransmitLevels.h
        Application.h
        Application.cpp
        Caffeine.cpp
        Caffeine.h
        ClientSettings.cpp
        ClientSettings.h
        EtcPalLogHandler.h
        EtcPalLogHandler.cpp
        Exception.h
        HandlerFactory.cpp
        HandlerFactory.h
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
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(libmobilesacn PRIVATE
            Caffeine_Windows.cpp
    )
else ()
    target_sources(libmobilesacn PRIVATE
            Caffeine_Other.cpp
    )
endif ()

find_package(sACN REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(libmobilesacn PRIVATE
        fmt::fmt
        httplib::httplib
        Qt::Core
        Qt::WebSockets
        # So we can get QApplication helpers not available in QCoreApplication.
        Qt::Widgets
        mobile_sacn_messages_cpp
        webui_qrc
        sACN
)
target_link_libraries(libmobilesacn PUBLIC
        spdlog::spdlog
        EtcPal
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
endif ()
target_compile_definitions(libmobilesacn PUBLIC CROW_ENFORCE_WS_SPEC CROW_ENABLE_COMPRESSION)
