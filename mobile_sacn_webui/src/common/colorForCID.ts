import {localStorageGet, LocalStorageItem} from "./localStorage.ts";
import CRC32 from "crc-32";
import Color from "colorjs.io";
import {ColorSpace} from "colorjs.io/fn";

const cidColorCache = {
    dark: new Map<string, Color>(),
    light: new Map<string, Color>(),
};

/**
 * Get a consistent color for a given CID.
 *
 * Borrowed from sACNView's algorithm, so it should match the desktop app.
 * @param cid
 */
export default function colorForCID(cid: string) {
    const darkMode = localStorageGet(LocalStorageItem.DARK_MODE, true);
    const colorCache = darkMode ? cidColorCache.dark : cidColorCache.light;
    const existing = colorCache.get(cid);
    if (existing !== undefined) {
        return existing;
    }
    const color = calcColorForCid(cid);
    colorCache.set(cid, color);
    return color;
}

function bytesFromCid(cid: string) {
    const cidHex = cid.split("-").join("");
    const bytes: number[] = [];
    for (let ix = 0; ix < cidHex.length;) {
        const byteHex = cidHex.substring(ix, ix += 2);
        const byte = parseInt(byteHex, 16);
        if (isNaN(byte)) {
            throw RangeError(`"${byteHex} is not convertable.`);
        }
        bytes.push(byte);
    }

    return new Uint8Array(bytes);
}

function calcColorForCid(cid: string) {
    // Get a checksum for a given CID.
    const uuid = bytesFromCid(cid);
    if (uuid === null) {
        console.error(`Error parsing CID ${cid}`);
        return new Color("white");
    }
    const id = CRC32.buf(uuid, 0) >>> 0;

    // Create a reasonable spread of different colors.
    const goldenRatio = 0.618033988749895;
    const hue = (goldenRatio * id) % 1.0;
    const sat = ((goldenRatio * id * 2) % 0.25) + 0.75;

    // Choose lightness based on color mode.
    const darkMode = localStorageGet(LocalStorageItem.DARK_MODE, true);
    const lightness = darkMode ? 0.25 : 0.5;

    return new Color("hsl", [
        hue * 360,
        sat * 100,
        lightness * 100,
    ]);
}
