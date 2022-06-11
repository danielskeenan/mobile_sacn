# Find the path to the Qt installation's bin directory.
function(find_qt_tool_path)
    if (NOT Qt${Qt_VERSION}_BIN)
        # Use the location of moc to find the absolute path to the tools directory.
        find_package(Qt${Qt_VERSION} COMPONENTS Core REQUIRED)
        get_target_property(_MOC_LOCATION Qt::moc IMPORTED_LOCATION)
        get_filename_component(_Qt_BIN "${_MOC_LOCATION}" DIRECTORY)
        set(Qt${Qt_VERSION}_BIN "${_Qt_BIN}" CACHE PATH "Path to Qt bin directory.")
    endif ()
    set(Qt_BIN "${Qt${Qt_VERSION}_BIN}" PARENT_SCOPE)
endfunction()

# Find a qt tool called NAME.  Creates an a cache variable called Qt<VERSION>_NAMEUPPER_COMMAND.
# Also creates an output variable without the Qt version called Qt_NAMEUPPER_COMMAND.
# Additional arguments are passed to the find_program() command.
# Example: find_qt_tool(windeployqt) creates a variable called Qt_WINDEPLOYQT_COMMAND.
function(find_qt_tool NAME)
    string(TOUPPER "${NAME}" NAME_UPPER)
    find_qt_tool_path()
    find_program(Qt${Qt_VERSION}_${NAME_UPPER}_COMMAND
        NAMES "${NAME}" "${NAME}${CMAKE_EXECUTABLE_SUFFIX}"
        HINTS "${Qt_BIN}"
        DOC "Path to Qt${Qt_VERSION} ${NAME}."
        ${ARGN}
        )
    set(Qt_${NAME_UPPER}_COMMAND "${Qt${Qt_VERSION}_${NAME_UPPER}_COMMAND}" PARENT_SCOPE)
endfunction()
