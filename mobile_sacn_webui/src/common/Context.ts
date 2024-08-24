import {createContext} from "react";
import {LevelDisplayMode} from "./levelDisplay.ts";

export interface AppContextProps {
    darkMode: boolean;
    setDarkMode: (darkMode: boolean) => void;
    levelDisplayMode: LevelDisplayMode;
    setLevelDisplayMode: (levelDisplayMode: LevelDisplayMode) => void;
}

const AppContext = createContext({} as AppContextProps);

export default AppContext;
