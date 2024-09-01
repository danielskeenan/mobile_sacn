export function* yieldFromGetterAndLength<T>(getter: (ix: number) => T, length: number) {
    for (let ix = 0; ix < length; ++ix) {
        yield getter(ix);
    }
}

/**
 * Similar to the C++ pattern of container constructors accepting a pointer to a buffer and the buffer's size.
 *
 * Used for extracting data from flatbuffer arrays.
 *
 * @param getter
 * @param length
 */
export function arrayFromGetterAndLength<T>(getter: (ix: number) => T, length: number) {
    return Array.from(yieldFromGetterAndLength(getter, length));
}
