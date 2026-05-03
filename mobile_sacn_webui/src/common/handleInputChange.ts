import clamp from "@/common/clamp";

/**
 * Check and handle a number input change event.
 *
 * Values will be clamped to within `min` and `max`.
 *
 * @param e Event.
 * @param min Minimum allowed value.
 * @param max Maximum allowed value.
 * @param currentValue The value before the event was fired.
 * @param setter Signal setter.
 */
export function handleInputNumberChange(e: Event, min: number, max: number, currentValue: number, setter: (val: number) => void) {
    const currentTarget = e.currentTarget as HTMLInputElement;
    const newVal = clamp(currentTarget.valueAsNumber, min, max);
    if (newVal === undefined) {
        currentTarget.valueAsNumber = currentValue;
    } else if (newVal != currentValue) {
        setter(newVal);
    }
}
