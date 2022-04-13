import {createContext} from "react";
import {Flash} from "./components/Flashes";
import {LevelDisplayMode} from "./components/LevelDisplay";

export interface AppContextProps {
    setFlashes: (flashes: Array<Flash>) => void;
    levelDisplayMode: LevelDisplayMode;
    setLevelDisplayMode: (levelDisplayMode: LevelDisplayMode) => void;
}

const AppContext = createContext({} as AppContextProps);

export default AppContext;
