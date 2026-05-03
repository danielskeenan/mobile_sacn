import "./LevelDisplay.scss";
import {useAppContext} from "@/common/AppContext";
import {LevelDisplayMode, levelDisplayString} from "@/common/levelDisplay";
import {type Component, mergeProps} from "solid-js";

interface LevelDisplayProps {
    level: number;
    displayMode?: LevelDisplayMode;
}

export const LevelDisplay: Component<LevelDisplayProps> = (props) => {
    const [appContext] = useAppContext();
    const mergedProps = mergeProps({displayMode: appContext.levelDisplayMode}, props);

    return (
        <span class="msacn-level">{levelDisplayString(mergedProps.level, mergedProps.displayMode)}</span>
    );
};

export const PriorityDisplay: Component<Omit<LevelDisplayProps, "displayMode">> = (props) => {
    return (
        <span class="msacn-priority">{levelDisplayString(props.level, LevelDisplayMode.DECIMAL)}</span>
    );
};
