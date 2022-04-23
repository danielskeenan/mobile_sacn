import "./LevelFader.scss";
import {Form} from "react-bootstrap";
import {LEVEL_MAX, LEVEL_MIN} from "../constants";
import React, {useCallback} from "react";
import {handleNumberFieldChange} from "../handleFieldChange";
import LevelDisplay from "./LevelDisplay";

interface LevelBarProps {
    label?: string;
    level: number;
    color?: string;
}

export function LevelBar(props: LevelBarProps) {
    const {label, level} = props;
    const percent = level / LEVEL_MAX * 100;

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <svg className="msacn-bar-container">
                <rect className="msacn-bar-bg" x="0" y="0" width="100%"/>
                <rect className="msacn-bar-filled" x="0" y="0" width={`${percent}%`} style={{fill: props.color}}/>
            </svg>
            <LevelDisplay level={level}/>
        </div>
    );
}

interface LevelFaderProps extends LevelBarProps {
    onLevelChange: (newValue: number) => void;
}

export function LevelFader(props: LevelFaderProps) {
    const {label, level, onLevelChange} = props;

    const onLevelFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onLevelChange(handleNumberFieldChange(e));
        },
        [onLevelChange]);

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <Form.Range value={level} onChange={onLevelFieldChange}
                        min={LEVEL_MIN} max={LEVEL_MAX}/>
            <LevelDisplay level={level}/>
        </div>
    );
}
