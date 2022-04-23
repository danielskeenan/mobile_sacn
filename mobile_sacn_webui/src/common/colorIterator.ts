const colors = [
    "#084298",
    "#801f4f",
    "#997404",
    "#087990",
    "#0f5132",
    "#842029",
    "#3d0a91",
    "#984c0c",
    "#13795b",
    "#432874",
];

/**
 * Generate a series of contrasting colors.
 */
export default function* colorIterator(): Generator<string> {
    let nextColorIx = 0;
    for(;;) {
        yield colors[nextColorIx];
        ++nextColorIx;
        if (nextColorIx >= colors.length) {
            nextColorIx = 0;
        }
    }
}
