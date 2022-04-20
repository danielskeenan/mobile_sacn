import {LEVEL_MAX, LEVEL_MIN, LEVEL_PERCENT_TABLE, PERCENT_LEVEL_TABLE} from "./constants";

describe("DMX percent tables match", () => {
    // Sanity check tables.
    for (let level = 0; level <= 255; ++level) {
        it(`Level/Percent table has entry for ${level}`, () => {
            expect(LEVEL_PERCENT_TABLE.has(level)).toEqual(true);
        });
    }
    for (let percent = 0; percent <= 100; ++percent) {
        it(`Percent/Level table has entry for ${percent}%`, () => {
            expect(PERCENT_LEVEL_TABLE.has(percent)).toEqual(true);
        });
    }

    // Check tables are symmetrical.
    // Don't check the Level/Percent table because the level is more precise than the percent;
    // there will always be mismatches.
    PERCENT_LEVEL_TABLE.forEach((level, percent) => {
        it(`Percent/Level table matches Level/Percent table for ${percent}%`, () => {
            expect(LEVEL_PERCENT_TABLE.get(level)).toEqual(percent);
        });
    });
});
