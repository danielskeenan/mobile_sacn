/**
 * Yields only the first instance of equal (`===`) entries.
 */
export default function* unique<T>(it: Iterable<T>): Generator<T, void, void> {
    const seen = new Set<T>();
    for (const entry of it) {
        if (!seen.has(entry)) {
            seen.add(entry);
            yield entry;
        }
    }
}
