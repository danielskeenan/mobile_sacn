# Generates rpc files.  Usage is similar to protobuf_generate_cpp.
function(proto_compile SRCS HDRS SCRIPTS)
    # Place for generated files.
    set(_PROTO_CPP_ROOT "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_proto")
    set(_PROTO_JS_ROOT "${PROJECT_SOURCE_DIR}/mobile_sacn_webui/src")

    set(_PROTO_FILES ${ARGN})
    if (NOT _PROTO_FILES)
        message(SEND_ERROR "Error: proto_compile() called without any proto files")
        return()
    endif ()

    # Find requirements.
    if (CMAKE_CROSSCOMPILING)
        find_program(_PROTOBUF_PROTOC protoc)
        #        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else ()
        find_package(Protobuf REQUIRED)
        set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
        #        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
    endif ()

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(_PROTOBUF_PROTOC_TYPESCRIPT_NAME "protoc-gen-ts.cmd")
    else ()
        set(_PROTOBUF_PROTOC_TYPESCRIPT_NAME "protoc-gen-ts")
    endif ()
    find_program(_PROTOBUF_PROTOC_TYPESCRIPT ${_PROTOBUF_PROTOC_TYPESCRIPT_NAME}
        PATHS ${PROJECT_SOURCE_DIR}/mobile_sacn_webui/node_modules/.bin
        REQUIRED
        )

    # Create a list of generated sources
    set(_GENERATED_SRCS)
    set(_GENERATED_HDRS)
    foreach (_PROTO ${_PROTO_FILES})
        # Handle protobuf files in subdirectories, unlike cmake's protobuf_generate_cpp.
        get_filename_component(_ABS_PATH ${_PROTO} ABSOLUTE)
        file(RELATIVE_PATH _REL_PATH ${PROJECT_SOURCE_DIR} ${_ABS_PATH})
        get_filename_component(_REL_DIR ${_REL_PATH} DIRECTORY)
        get_filename_component(_BASENAME ${_PROTO} NAME_WE)
        set(_THIS_PROTO_CPP_ROOT "${_PROTO_CPP_ROOT}/${_REL_DIR}")
        set(_THIS_PROTO_JS_ROOT "${_PROTO_JS_ROOT}/${_REL_DIR}")
        set(_THIS_SRC "${_THIS_PROTO_CPP_ROOT}/${_BASENAME}.pb.cc")
        set(_THIS_HDR "${_THIS_PROTO_CPP_ROOT}/${_BASENAME}.pb.h")
        # The code generator appends ".grpc" to the file's base name.
        #        set(_THIS_GRPC_SRC "${_THIS_PROTO_CPP_ROOT}/${_BASENAME}.grpc.pb.cc")
        #        set(_THIS_GRPC_HDR "${_THIS_PROTO_CPP_ROOT}/${_BASENAME}.grpc.pb.h")

        # Append to this list of generated files.
        list(APPEND _GENERATED_SRCS "${_THIS_SRC}")
        list(APPEND _GENERATED_HDRS "${_THIS_HDR}")
        #        list(APPEND _GENERATED_SRCS "${_THIS_GRPC_SRC}")
        #        list(APPEND _GENERATED_HDRS "${_THIS_GRPC_HDR}")

        # Add the build step
        file(MAKE_DIRECTORY "${_THIS_PROTO_CPP_ROOT}")
        file(MAKE_DIRECTORY "${_THIS_PROTO_JS_ROOT}")
        add_custom_command(
            OUTPUT ${_THIS_SRC} ${_THIS_HDR}
            COMMAND "${_PROTOBUF_PROTOC}"
            ARGS
            --plugin="protoc-gen-ts=${_PROTOBUF_PROTOC_TYPESCRIPT}"
            --cpp_out="${_THIS_PROTO_CPP_ROOT}"
            --ts_out="${_THIS_PROTO_JS_ROOT}"
            --proto_path="${CMAKE_CURRENT_SOURCE_DIR}"
            ${_PROTO}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${_PROTO}
        )
    endforeach ()
    set(${SRCS} ${_GENERATED_SRCS} PARENT_SCOPE)
    set(${HDRS} ${_GENERATED_HDRS} PARENT_SCOPE)
endfunction()
