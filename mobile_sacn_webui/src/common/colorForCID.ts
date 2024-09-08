import CRC32 from "crc-32";
import Color from "colorjs.io";

interface CidColor {
    light: Color;
    dark: Color;
}

const cidColorCache = new Map<string, CidColor>();

/**
 * Get a consistent color for a given CID.
 *
 * Borrowed from sACNView's algorithm, so it should match the desktop app.
 * @param cid
 */
export default function colorForCID(cid: string) {
    const existing = cidColorCache.get(cid);
    if (existing !== undefined) {
        return existing;
    }
    const color = calcColorForCid(cid);
    cidColorCache.set(cid, color);
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

function calcColorForCid(cid: string): CidColor {
    // Get a checksum for a given CID.
    const uuid = bytesFromCid(cid);
    if (uuid === null) {
        console.error(`Error parsing CID ${cid}`);
        return {
            light: new Color("white"),
            dark: new Color("black"),
        };
    }
    const id = CRC32.buf(uuid, 0) >>> 0;

    // Create a reasonable spread of different colors.
    const goldenRatio = 0.618033988749895;
    const hue = (goldenRatio * id) % 1.0;
    const sat = ((goldenRatio * id * 2) % 0.25) + 0.75;

    return {
        light: new Color("hsl", [
            hue * 360,
            sat * 100,
            50,
        ]),
        dark: new Color("hsl", [
            hue * 360,
            sat * 100,
            25,
        ]),
    };
}
