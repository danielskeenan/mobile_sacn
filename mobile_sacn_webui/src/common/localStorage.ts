export enum LocalStorageItem {
    LEVEL_DISPLAY_MODE = "level_display_mode",
}

/**
 * Get a value from localStorage.  Set it to defaultValue if it is not set already.
 * @param key
 * @param defaultValue
 */
export function localStorageGet(key: LocalStorageItem, defaultValue?: string): string {
    const value = localStorage.getItem(key);
    if (value === null) {
        if (defaultValue !== undefined) {
            localStorage.setItem(key, defaultValue);
            return defaultValue;
        }
        return "";
    }

    return value;
}

/**
 * Set a value in localStorage,
 * @param key
 * @param value
 */
export function localStorageSet(key: LocalStorageItem, value: string) {
    localStorage.setItem(key, value);
}
