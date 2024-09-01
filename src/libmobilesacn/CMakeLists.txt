add_library(libmobilesacn STATIC
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/ChanCheck.h
    rpc/ChanCheck.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/ReceiveLevels.h
    rpc/ReceiveLevels.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/RpcHandler.h
    rpc/RpcHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/TransmitHandler.h
    rpc/TransmitHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/rpc/TransmitLevels.h
    rpc/TransmitLevels.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/Application.h
    Application.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/CrowLogHandler.h
    CrowLogHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/EtcPalLogHandler.h
    EtcPalLogHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/Exception.h
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/HttpServer.h
    HttpServer.cpp
    ${PROJECT_SOURCE_DIR}/include/libmobilesacn/SacnCidGenerator.h
    SacnCidGenerator.cpp
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
target_link_libraries(libmobilesacn PUBLIC spdlog::spdlog EtcPal)
if  (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(libmobilesacn PUBLIC ${CMAKE_DL_LIBS})
endif ()
target_compile_definitions(libmobilesacn PUBLIC CROW_ENFORCE_WS_SPEC CROW_ENABLE_COMPRESSION)
