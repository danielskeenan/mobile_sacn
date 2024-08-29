import {DMX_MAX, DMX_MIN, LEVEL_MAX, LEVEL_MIN} from "../../common/constants";
import {localStorageGet, LocalStorageItem} from "../../common/localStorage";
import {LEVEL_PERCENT_TABLE, LevelDisplayMode, PERCENT_LEVEL_TABLE} from "../../common/levelDisplay.ts";
import {clamp} from "lodash";

export enum CmdLineTokenType {
    NUMBER,
    HEX_DIGIT,
    PLUS,
    MINUS,
    THRU,
    AT,
    ENTER,
}

export abstract class CmdLineToken {
    abstract type(): CmdLineTokenType;

    abstract toString(): string;
}

export class CmdLineTokenNumber extends CmdLineToken {
    value: string;

    constructor(value: string) {
        super();
        this.value = value;
    }

    type(): CmdLineTokenType {
        return CmdLineTokenType.NUMBER;
    }

    toString(): string {
        return this.value;
    }
}

export class CmdLineTokenHexDigit extends CmdLineTokenNumber {
    type(): CmdLineTokenType {
        return CmdLineTokenType.HEX_DIGIT;
    }
}

export class CmdLineTokenPlus extends CmdLineToken {
    type(): CmdLineTokenType {
        return CmdLineTokenType.PLUS;
    }

    toString(): string {
        return "+";
    }
}

export class CmdLineTokenMinus extends CmdLineToken {
    type(): CmdLineTokenType {
        return CmdLineTokenType.MINUS;
    }

    toString(): string {
        // This is a minus sign (U+2212), not a dash!
        return "−";
    }
}

export class CmdLineTokenThru extends CmdLineToken {
    type(): CmdLineTokenType {
        return CmdLineTokenType.THRU;
    }

    toString(): string {
        return "Thru";
    }
}

export class CmdLineTokenAt extends CmdLineToken {
    type(): CmdLineTokenType {
        return CmdLineTokenType.AT;
    }

    toString(): string {
        return "@";
    }
}

export class CmdLineTokenEnter extends CmdLineToken {
    type(): CmdLineTokenType {
        return CmdLineTokenType.ENTER;
    }


    toString(): string {
        return "*";
    }
}

/**
 * Determine which tokens are allowed next on the command line.
 *
 * @param cmdline
 */
export function allowedTokens(cmdline: CmdLineToken[]) {
    let nextAllowed = [CmdLineTokenType.NUMBER];
    if (cmdline.length === 0) {
        // The first token must always be a number.
        return nextAllowed;
    }

    // Need to know if we're in the selection part or the level part.
    let at = false;
    for (const token of cmdline) {
        if (token instanceof CmdLineTokenAt) {
            at = true;
            break;
        }
    }
    const lastToken = cmdline.at(-1);
    if (lastToken instanceof CmdLineTokenNumber) {
        if (!at) {
            // Selection
            nextAllowed = [CmdLineTokenType.NUMBER, CmdLineTokenType.PLUS, CmdLineTokenType.MINUS, CmdLineTokenType.THRU, CmdLineTokenType.AT];
        } else {
            // Level
            nextAllowed = [CmdLineTokenType.NUMBER, CmdLineTokenType.THRU];
        }
    } else if (lastToken instanceof CmdLineTokenPlus || lastToken instanceof CmdLineTokenMinus || lastToken instanceof CmdLineTokenThru) {
        nextAllowed = [CmdLineTokenType.NUMBER];
    } else if (lastToken instanceof CmdLineTokenAt) {
        // Level
        nextAllowed = [CmdLineTokenType.NUMBER, CmdLineTokenType.PLUS, CmdLineTokenType.MINUS];
    }
    if (at) {
        // Allow hex digits only for levels.
        nextAllowed.push(CmdLineTokenType.HEX_DIGIT);
    }

    return nextAllowed;
}

/**
 * Is this a complete command line?
 *
 * @param cmdline
 */
export function cmdLineIsComplete(cmdline: CmdLineToken[]) {
    if (cmdline.length === 0) {
        return false;
    }

    let cmdlineIx = 0;
    let at = false;
    let level = false;
    for (; cmdlineIx < cmdline.length; ++cmdlineIx) {
        const token = cmdline[cmdlineIx];
        if (token instanceof CmdLineTokenAt) {
            at = true;
            break;
        }
    }
    for (; cmdlineIx < cmdline.length; ++cmdlineIx) {
        const token = cmdline[cmdlineIx];
        level = (token instanceof CmdLineTokenNumber);
    }

    return at && level;
}

/**
 * Update the levels parameter using the given command line.
 *
 * @param cmdline
 * @param levels
 */
export function updateLevelsFromCmdLine(cmdline: CmdLineToken[], levels: number[]): number[] {
    if (cmdline.length === 0) {
        return levels;
    }

    let cmdlineIx = 0;
    // Determine the addresses to change.
    const selection = new Set<number>();
    let plus = true;
    let minus = false;
    let thru = false;
    let lastNumber = 1;
    for (; cmdlineIx < cmdline.length; ++cmdlineIx) {
        const token = cmdline[cmdlineIx];
        if (token instanceof CmdLineTokenNumber) {
            const tokenValue = parseInt(token.value);
            const rangeStart = Math.max(thru ? lastNumber : tokenValue, DMX_MIN);
            const rangeEnd = Math.min(tokenValue, DMX_MAX);
            if (plus) {
                for (let addr = rangeStart; addr <= rangeEnd; addr += 1) {
                    selection.add(addr);
                }
            } else if (minus) {
                selection.forEach((addr) => {
                    if (addr >= rangeStart && addr <= rangeEnd) {
                        selection.delete(addr);
                    }
                });
            }
            lastNumber = tokenValue;
        } else if (token instanceof CmdLineTokenPlus) {
            plus = true;
            minus = false;
            thru = false;
        } else if (token instanceof CmdLineTokenMinus) {
            plus = false;
            minus = true;
            thru = false;
        } else if (token instanceof CmdLineTokenThru) {
            thru = true;
        } else if (token instanceof CmdLineTokenAt) {
            ++cmdlineIx;
            break;
        }
    }

    // Determine the level change.
    plus = false;
    minus = false;
    thru = false;
    let levelRange: number[] = [];
    const levelDisplayMode = localStorageGet(LocalStorageItem.LEVEL_DISPLAY_MODE, LevelDisplayMode.PERCENT) as LevelDisplayMode;
    for (; cmdlineIx < cmdline.length; ++cmdlineIx) {
        const token = cmdline[cmdlineIx];
        if (token instanceof CmdLineTokenNumber) {
            if (!thru) {
                levelRange = [parseInt(token.value, levelDisplayMode == LevelDisplayMode.HEX ? 16 : 10)];
            } else {
                // Allows for things like 1 thru 5 at 10 thru 50 => 1@10, 2@20, 3@30, 4@40, 5@50
                const levelStart = levelRange[0];
                levelRange = [];
                if (selection.size > 1) {
                    const levelEnd = parseInt(token.value, levelDisplayMode == LevelDisplayMode.HEX ? 16 : 10);
                    const levelChange = (levelEnd - levelStart) / (selection.size - 1);
                    for (let level = levelStart; levelRange.length < selection.size; level += levelChange) {
                        levelRange.push(level);
                    }
                }
            }
        } else if (token instanceof CmdLineTokenPlus) {
            plus = true;
            minus = false;
            thru = false;
        } else if (token instanceof CmdLineTokenMinus) {
            plus = false;
            minus = true;
            thru = false;
        } else if (token instanceof CmdLineTokenThru) {
            plus = false;
            minus = false;
            thru = true;
        }
    }

    // Update levels.
    if (levelRange.length === 0) {
        // Do nothing, at enter does nothing in the program.
        return levels;
    }
    let levelIx = 0;
    selection.forEach((addr) => {
        // Working with DMX (0-255) can cause off-by-one errors when the user is thinking in percentages.
        let currentLevel = convertActualLevelToUserLevel(levels[addr - 1]);
        let levelChange = levelRange[levelIx];
        let newLevel;
        if (plus) {
            newLevel = currentLevel + levelChange;
        } else if (minus) {
            newLevel = currentLevel - levelChange;
        } else {
            newLevel = levelChange;
        }
        ++levelIx;
        if (levelIx >= levelRange.length) {
            // Loop level range.
            levelIx = 0;
        }

        // Only set the level if it fits in a DMX stream.
        if (addr >= DMX_MIN && addr <= DMX_MAX) {
            levels[addr - 1] = clamp(convertUserLevelToActualLevel(newLevel), LEVEL_MIN, LEVEL_MAX);
        }
    });

    return levels;
}

/**
 * Convert a level as the user sees it (may be a percent) to actual 0-255 level.
 * @param level
 */
function convertUserLevelToActualLevel(level: number): number {
    const levelDisplayMode = localStorageGet(LocalStorageItem.LEVEL_DISPLAY_MODE, LevelDisplayMode.PERCENT) as LevelDisplayMode;
    if (levelDisplayMode === LevelDisplayMode.PERCENT) {
        let actualLevel = PERCENT_LEVEL_TABLE[level];
        if (actualLevel === undefined) {
            // The level is some decimal percentage value.
            actualLevel = Math.round(level / 100 * 255);
        }
        return actualLevel;
    }

    return level;
}

/**
 * Convert an actual 0-255 level to a level as the user sees it (may be a percent).
 * @param level
 */
function convertActualLevelToUserLevel(level: number): number {
    const levelDisplayMode = localStorageGet(LocalStorageItem.LEVEL_DISPLAY_MODE, LevelDisplayMode.PERCENT) as LevelDisplayMode;
    if (levelDisplayMode === LevelDisplayMode.PERCENT) {
        return LEVEL_PERCENT_TABLE[level] ?? 0;
    }

    return level;
}
