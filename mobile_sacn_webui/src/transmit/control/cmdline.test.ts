import {
    allowedTokens,
    cmdLineIsComplete,
    CmdLineTokenAt,
    CmdLineTokenMinus,
    CmdLineTokenNumber,
    CmdLineTokenPlus,
    CmdLineTokenThru,
    CmdLineTokenType,
    updateLevelsFromCmdLine,
} from "./cmdline";
import {PERCENT_LEVEL_TABLE} from "../../common/constants";

describe("Allowed next token makes sense", () => {
    it("(empty command line)", () => {
        expect(allowedTokens([])).toIncludeSameMembers([CmdLineTokenType.NUMBER]);
    });

    it("1", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER, CmdLineTokenType.PLUS, CmdLineTokenType.MINUS, CmdLineTokenType.THRU, CmdLineTokenType.AT]);
    });

    it("1 +", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenPlus(),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER]);
    });
    it("1 -", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenMinus(),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER]);
    });
    it("1 Thru", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER]);
    });

    it("1 @", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenAt(),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER, CmdLineTokenType.PLUS, CmdLineTokenType.MINUS]);
    });
    it("1 @ 10", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("10"),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER, CmdLineTokenType.THRU]);
    });
    it("1 @ 10 Thru", () => {
        expect(allowedTokens([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenThru(),
        ])).toIncludeSameMembers([CmdLineTokenType.NUMBER]);
    });
});

describe("Parser knows when command line is complete", () => {
    it("(empty command line)", () => {
        expect(cmdLineIsComplete([])).toBeFalse();
    });

    it("1", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
        ])).toBeFalse();
    });

    it("1 +", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenPlus(),
        ])).toBeFalse();
    });
    it("1 -", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenMinus(),
        ])).toBeFalse();
    });
    it("1 Thru", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
        ])).toBeFalse();
    });

    it("1 Thru 10", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
        ])).toBeFalse();
    });

    it("1 Thru 10 @", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
        ])).toBeFalse();
    });

    it("1 Thru 10 @ 5", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("5"),
        ])).toBeTrue();
    });

    it("1 Thru 10 @ 5 Thru", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenThru(),
        ])).toBeFalse();
    });

    it("1 Thru 10 @ 5 Thru 10", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
        ])).toBeTrue();
    });

    it("1 Thru 10 @ +", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenPlus(),
        ])).toBeFalse();
    });

    it("1 Thru 10 @ -", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenMinus(),
        ])).toBeFalse();
    });

    it("1 Thru 10 @ + 10", () => {
        expect(cmdLineIsComplete([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenAt(),
            new CmdLineTokenPlus(),
            new CmdLineTokenNumber("10"),
        ])).toBeTrue();
    });
});

describe("Levels updated properly", () => {
    // Full buffer with all levels set to 0.
    const levelsZero = Array.from({length: 512}, () => 0);

    it("(empty command line)", () => {
        expect(updateLevelsFromCmdLine([], [...levelsZero])).toEqual(levelsZero);
    });

    // Because there is no localstorage available, the level setting on the command line specifies percentages.
    it("1 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 + 2 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[1] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenPlus(),
            new CmdLineTokenNumber("2"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 Thru 5 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[1] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[2] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[3] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[4] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 Thru 5 - 3 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[1] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[3] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[4] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenMinus(),
            new CmdLineTokenNumber("3"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 Thru 5 + 11 Thru 15 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[1] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[2] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[3] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[4] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[10] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[11] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[12] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[13] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[14] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenPlus(),
            new CmdLineTokenNumber("11"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("15"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 Thru 10 - 2 Thru 4 @ 100", () => {
        const expected = [...levelsZero];
        expected[0] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[4] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[5] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[6] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[7] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[8] = PERCENT_LEVEL_TABLE.get(100) as number;
        expected[9] = PERCENT_LEVEL_TABLE.get(100) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("10"),
            new CmdLineTokenMinus(),
            new CmdLineTokenNumber("2"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("4"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("100"),
        ], [...levelsZero])).toEqual(expected);
    });

    it("1 Thru 5 @ + 10", () => {
        // Start with all at 10%
        const before = Array.from({length: 512}, () => PERCENT_LEVEL_TABLE.get(10) as number);
        const expected = [...before];
        expected[0] = PERCENT_LEVEL_TABLE.get(20) as number;
        expected[1] = PERCENT_LEVEL_TABLE.get(20) as number;
        expected[2] = PERCENT_LEVEL_TABLE.get(20) as number;
        expected[3] = PERCENT_LEVEL_TABLE.get(20) as number;
        expected[4] = PERCENT_LEVEL_TABLE.get(20) as number;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenAt(),
            new CmdLineTokenPlus(),
            new CmdLineTokenNumber("10"),
        ], before)).toEqual(expected);
    });

    it("1 Thru 5 @ - 10", () => {
        // Start with all at 10%
        const before = Array.from({length: 512}, () => PERCENT_LEVEL_TABLE.get(10) as number);
        const expected = [...before];
        expected[0] = 0;
        expected[1] = 0;
        expected[2] = 0;
        expected[3] = 0;
        expected[4] = 0;
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
            new CmdLineTokenAt(),
            new CmdLineTokenMinus(),
            new CmdLineTokenNumber("10"),
        ], before)).toEqual(expected);
    });

    it("1 Thru 11 @ 1 thru 5", () => {
        // Start with all at 10%
        const before = Array.from({length: 512}, () => PERCENT_LEVEL_TABLE.get(10) as number);
        const expected = [...before];
        expected[0] = Math.round(1 / 100 * 255);
        expected[1] = Math.round(1.4 / 100 * 255);
        expected[2] = Math.round(1.8 / 100 * 255);
        expected[3] = Math.round(2.2 / 100 * 255);
        expected[4] = Math.round(2.6 / 100 * 255);
        expected[5] = Math.round(3 / 100 * 255);
        expected[6] = Math.round(3.4 / 100 * 255);
        expected[7] = Math.round(3.8 / 100 * 255);
        expected[8] = Math.round(4.2 / 100 * 255);
        expected[9] = Math.round(4.6 / 100 * 255);
        expected[10] = Math.round(5 / 100 * 255);
        expect(updateLevelsFromCmdLine([
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("11"),
            new CmdLineTokenAt(),
            new CmdLineTokenNumber("1"),
            new CmdLineTokenThru(),
            new CmdLineTokenNumber("5"),
        ], before)).toEqual(expected);
    });
});
