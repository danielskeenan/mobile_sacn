include(${PROJECT_SOURCE_DIR}/cmake/proto_compile.cmake)
proto_compile(_SRCS _HDRS _SCRIPTS
    chan_check.proto
    control.proto
    effect.proto
    view_levels.proto
    )

add_library(mobilesacnproto ${_SRCS} ${_HDRS})

find_package(Protobuf COMPONENTS libprotobuf REQUIRED)
target_link_libraries(mobilesacnproto PUBLIC protobuf::libprotobuf)
set_property(TARGET mobilesacnproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}_proto)
