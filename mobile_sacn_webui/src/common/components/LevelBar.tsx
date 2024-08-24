import "./LevelBar.scss";
import {LEVEL_MAX, LEVEL_MIN} from "../constants.ts";
import LevelDisplay from "./LevelDisplay.tsx";
import {Form} from "react-bootstrap";

interface LevelBarProps {
    label?: string;
    level: string;
    color?: string;
}

export function LevelBar(props: LevelBarProps) {
    const {label, level} = props;
    const percent = Number(level) / LEVEL_MAX * 100;

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <svg className="msacn-bar-container">
                <rect className="msacn-bar-bg" x="0" y="0" width="100%"/>
                <rect className="msacn-bar-filled" x="0" y="0" width={`${percent}%`} style={{fill: props.color}}/>
            </svg>
            <LevelDisplay level={Number(level)}/>
        </div>
    );
}

interface LevelFaderProps extends LevelBarProps {
    onLevelChange: (newValue: string) => void;
}

export function LevelFader(props: LevelFaderProps) {
    const {label, level} = props;

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <Form.Range value={level} onChange={e => props.onLevelChange(e.target.value)}
                        min={LEVEL_MIN} max={LEVEL_MAX}/>
            <LevelDisplay level={Number(level)}/>
        </div>
    );
}
