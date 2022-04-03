/**
 * Return true if min <= value <= max.
 * @param value
 * @param min
 * @param max
 */
export default function inRange(value: Number, min: Number, max: Number): boolean {
    return value >= min && value <= max;
}
