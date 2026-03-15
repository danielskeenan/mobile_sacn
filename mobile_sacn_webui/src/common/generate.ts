/**
 * Yield @p value @p count times.
 * @param count
 * @param value
 */
export function* generate<T>(count: number, value: T): Generator<T, void, void> {
    for (let i = 0; i < count; ++i) {
        yield value;
    }
}
