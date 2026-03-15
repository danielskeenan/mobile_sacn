/**
 * Yield the range `[begin, end)`.
 * @param begin
 * @param end
 * @param increment
 */
export default function* iota(begin: number, end: number, increment: number = 1): Generator<number, void, void> {
    for (let i = begin; i < end; i += increment) {
        yield i;
    }
}
