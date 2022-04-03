import {createContext} from "react";
import {Flash} from "./components/Flashes";

export interface AppContextProps {
    setFlashes?: (flashes: Array<Flash>) => void;
}

const AppContext = createContext({} as AppContextProps);

export default AppContext;
