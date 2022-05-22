# Compute the SHA256 hash for the given files.
# Usage: hashfiles(<variable> <file 1> [<file 2>...])
function(hashfiles OUTPUT_VARIABLE)
    set(INPUT_FILES "${ARGN}")
    set(INPUT_FILES_ABS)
    set(HASHES)

    # Sort the input files so they appear in the same order each time.  This works around
    # filesystems returning the same files in a different order changing the hash.
    foreach (FILE ${INPUT_FILES})
        # File hashing requires an absolute path.
        file(REAL_PATH "${FILE}" FILE_ABS)
        list(APPEND INPUT_FILES_ABS "${FILE_ABS}")
    endforeach ()
    list(SORT INPUT_FILES_ABS)

    # Compute the hash for each file.
    foreach (FILE ${INPUT_FILES_ABS})
        file(SHA256 "${FILE_ABS}" HASH)
        list(APPEND HASHES "${HASH}")
    endforeach ()

    string(SHA256 HASH "${HASHES}")
    set(${OUTPUT_VARIABLE} "${HASH}" PARENT_SCOPE)
endfunction()
