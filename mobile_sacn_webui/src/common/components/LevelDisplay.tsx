import "./LevelDisplay.scss";
import {LevelDisplayMode, levelDisplayString} from "../levelDisplay.ts";
import AppContext from "../Context.ts";
import {useContext} from "react";

interface LevelDisplayProps {
    level: number;
    displayMode?: LevelDisplayMode;
}

export default function LevelDisplay(props: LevelDisplayProps) {
    const {level, displayMode} = {
        ...props, ...{
            displayMode: useContext(AppContext).levelDisplayMode,
        },
    };

    return (
        <span className="msacn-level">{levelDisplayString(level, displayMode)}</span>
    );
}
