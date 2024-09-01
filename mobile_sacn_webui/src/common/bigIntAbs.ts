/**
 * Same usage as Math.abs, except for bigint.
 * @param val
 */
export default function bigIntAbs(val: bigint) {
    return val < 0 ? -val : val;
}
