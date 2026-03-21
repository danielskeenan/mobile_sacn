/**
 * Clamp @p val between @p min and @p max inclusive.
 *
 * Returns @p defVal (or `undefined` if not passed) if @p val is NaN or infinite.
 */
export default function clamp(val: number, min: number, max: number, defVal?: number) {
    if (Number.isNaN(val) || !Number.isFinite(val)) {
        return defVal;
    } else if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }
    return val;
}
