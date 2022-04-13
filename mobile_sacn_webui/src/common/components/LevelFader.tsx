import "./LevelFader.scss";
import {Form} from "react-bootstrap";
import {LEVEL_MAX, LEVEL_MIN} from "../constants";
import React, {useCallback} from "react";
import {handleNumberFieldChange} from "../handleFieldChange";
import LevelDisplay from "./LevelDisplay";

interface LevelFaderProps {
    level: number;
    onLevelChange: (newValue: number) => void;
}

export default function LevelFader(props: LevelFaderProps) {
    const {level, onLevelChange} = props;

    const onLevelFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onLevelChange(handleNumberFieldChange(e));
        },
        [onLevelChange]);

    return (
        <div className="msacn-fader">
            <Form.Range value={level} onChange={onLevelFieldChange}
                        min={LEVEL_MIN} max={LEVEL_MAX}/>
            <LevelDisplay level={level}/>
        </div>
    );
}
