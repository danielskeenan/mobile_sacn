export enum LocalStorageItem {
    // Development override, stores path to the backend. No way to set this from UI.
    SERVER = "server",
    DARK_MODE = "dark_mode",
    LEVEL_DISPLAY_MODE = "level_display_mode",
}

/**
 * Get a value from localStorage.  Set it to defaultValue if it is not set already.
 * @param key
 * @param defaultValue
 */
export function localStorageGet<T, D extends T = T>(key: LocalStorageItem, defaultValue: D): T {
    const value = localStorage.getItem(key);
    if (value === null) {
        localStorageSet(key, defaultValue);
        return defaultValue;
    }

    try {
        return JSON.parse(value);
    } catch (e) {
        if (e instanceof SyntaxError) {
            // Bad LocalStorage value.
            localStorageSet(key, defaultValue);
            return defaultValue;
        }
        throw e;
    }
}

/**
 * Set a value in localStorage,
 * @param key
 * @param value
 */
export function localStorageSet<T>(key: LocalStorageItem, value: T): void {
    localStorage.setItem(key, JSON.stringify(value));
}
