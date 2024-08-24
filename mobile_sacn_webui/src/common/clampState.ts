/**
 * If val does not meet min <= val <= max, call setter as follows:
 *
 * - If val < min, calls setter(min)
 * - If val > max, calls setter (max)
 * - If val is invalid (i.e. is NaN or infinite), call setter("").
 *
 * This function is useful for numeric input fields where keyboard input is not validated by the browser.
 *
 * @param val
 * @param setter
 * @param min
 * @param max
 */
export default function clampState(val: string, setter: (newVal: string) => void, min: number, max: number) {
    const intVal = Number(val);
    if (Number.isNaN(intVal) || !Number.isFinite(intVal)) {
        setter("");
    } else if (intVal < min) {
        setter(min.toString());
    } else if (intVal > max) {
        setter(max.toString());
    }
}
